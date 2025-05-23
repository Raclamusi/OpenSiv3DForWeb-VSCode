mergeInto(LibraryManager.library, {
    siv3dDecodeAudioFromFileAsync: function(filePath, callback, arg) {
        const path = UTF8ToString(filePath, 1024);
        const fileBytes = FS.readFile(path);

        const onSuccess = function(decoded) {
            const leftDataBuffer = Module["_malloc"](decoded.length * 4);
            HEAPF32.set(decoded.getChannelData(0), leftDataBuffer>>2);

            let rightDataBuffer;
            
            if (decoded.numberOfChannels >= 2) {
                rightDataBuffer = Module["_malloc"](decoded.length * 4);
                HEAPF32.set(decoded.getChannelData(1), rightDataBuffer>>2);
            } else {
                rightDataBuffer = leftDataBuffer;
            }

            HEAP32[(arg>>2)+0] = leftDataBuffer;
            HEAP32[(arg>>2)+1] = rightDataBuffer;
            HEAPU32[(arg>>2)+2] = decoded.sampleRate;
            HEAPU32[(arg>>2)+3] = decoded.length;

            {{{ makeDynCall('vi', 'callback') }}}(arg);
            _siv3dMaybeAwake();
        };

        const onFailure = function() {
            HEAP32[(arg>>2)+0] = 0;
            HEAP32[(arg>>2)+1] = 0;
            HEAPU32[(arg>>2)+2] = 0;
            HEAPU32[(arg>>2)+3] = 0;

            {{{ makeDynCall('vi', 'callback') }}}(arg);
            _siv3dMaybeAwake();
        }

        Module["SDL2"].audioContext.decodeAudioData(fileBytes.buffer, onSuccess, onFailure);   
    },
    siv3dDecodeAudioFromFileAsync__sig: "viii",
    siv3dDecodeAudioFromFileAsync__deps: [ "$AL", "$FS", "siv3dMaybeAwake" ],
});
mergeInto(LibraryManager.library, {
    siv3dSetClipboardText: function(ctext) {
        const text = UTF8ToString(ctext);
        
        siv3dRegisterUserAction(function () {
            navigator.clipboard.writeText(text);
        });
    },
    siv3dSetClipboardText__sig: "vi",
    siv3dSetClipboardText__proxy: "sync",
    siv3dSetClipboardText__deps: [ "$siv3dRegisterUserAction" ],

    $siv3dGetClipboardTextImpl: function(wakeUp) {
        if (!navigator.clipboard.readText) {
            err("Reading clipboard is not allowed in this browser.");
            wakeUp(0);
            return;
        }

        siv3dRegisterUserAction(function () {
            navigator.clipboard.readText()
                .then(function(str) {
                    const strPtr = allocate(intArrayFromString(str), ALLOC_NORMAL);       
                    wakeUp(strPtr);
                })
                .catch(function(_) {
                    wakeUp(0);
                });
        });
    },
    $siv3dGetClipboardTextImpl__deps: [ "$siv3dRegisterUserAction" ],

#if ASYNCIFY
    siv3dGetClipboardText: function() {
        return Asyncify.handleSleep(siv3dGetClipboardTextImpl);
    },
    siv3dGetClipboardText__sig: "iv",
    siv3dGetClipboardText__deps: [ "$siv3dGetClipboardTextImpl", "$Asyncify" ],
#elif PROXY_TO_PTHREAD
    siv3dGetClipboardText: function(ctx) {
        siv3dGetClipboardTextImpl(function () {
            Module["_emscripten_proxy_finish"](ctx);
        });
    },
    siv3dGetClipboardText__sig: "ii",
    siv3dGetClipboardText__deps: [ "$siv3dGetClipboardTextImpl" ],
#else
    siv3dGetClipboardText: function() {
        return 0;
    },
    siv3dGetClipboardText__sig: "iv",
#endif

    siv3dGetClipboardTextAsync: function(callback, promise) {
        siv3dRegisterUserAction(function () {
            if (!navigator.clipboard.readText) {
                err("Reading clipboard is not allowed in this browser.");
                {{{ makeDynCall('vii', 'callback') }}}(0, promise);
                return;
            }

            navigator.clipboard.readText()
            .then(function(str) {
                const strPtr = allocate(intArrayFromString(str), ALLOC_NORMAL);       
                {{{ makeDynCall('vii', 'callback') }}}(strPtr, promise);
                Module["_free"](strPtr);
            })
            .catch(function (e) {
                {{{ makeDynCall('vii', 'callback') }}}(0, promise);
            });
        });
    },
    siv3dGetClipboardTextAsync__sig: "vii",
    siv3dGetClipboardTextAsync__deps: [ "$siv3dRegisterUserAction" ],
});mergeInto(LibraryManager.library, { 
    $siv3dInputElement: null,
    $siv3dDialogFileReader: null,
    $siv3dDownloadLink: null,

    siv3dInitDialog: function() {
        siv3dInputElement = document.createElement("input");
        siv3dInputElement.type = "file";

        siv3dDialogFileReader = new FileReader();
        siv3dDownloadLink = document.createElement("a");
    },
    siv3dInitDialog__sig: "v",
    siv3dInitDialog__deps: [ "$siv3dInputElement", "$siv3dDialogFileReader", "$siv3dDownloadLink" ],

    $siv3dOpenDialogAsync: function(filter, callback, futurePtr, acceptMuilitple) {
        siv3dInputElement.accept = filter;
        siv3dInputElement.multiple = acceptMuilitple;

        function cancelHandler(e) {
            {{{ makeDynCall('viii', 'callback') }}}(0, 0, futurePtr);
            _siv3dMaybeAwake();
        }

        // Using addEventListener works in Firefox
        // Set 'once' to automatically delete the handler when the event fires
        siv3dInputElement.addEventListener('cancel', cancelHandler, { once: true });

        siv3dInputElement.oninput = async function(e) {
            // Delete event handler if cancel event is not fired
            siv3dInputElement.removeEventListener('cancel', cancelHandler);

            // Workaround for event firing
            const files = [...e.target.files];
            e.target.value = '';

            if (files.length < 1) {
                callback(0, 0, futurePtr);
                _siv3dMaybeAwake();
                return;
            }

            let filePathes = [];
            const sp = stackSave();

            for (let file of files) {
                await new Promise((resolve) => {
                    const filePath = "/tmp/" + file.name;

                    siv3dDialogFileReader.addEventListener("load", function onLoaded() {
                        FS.writeFile(filePath, new Uint8Array(siv3dDialogFileReader.result));
        
                        const namePtr = allocate(intArrayFromString(filePath), ALLOC_STACK);
                        filePathes.push(namePtr);
                        
                        siv3dDialogFileReader.removeEventListener("load", onLoaded);
                        resolve();
                    });
        
                    siv3dDialogFileReader.readAsArrayBuffer(file);  
                });
            }

            const filePathesPtr = stackAlloc(filePathes.length * {{{ POINTER_SIZE }}});

            for (let i = 0; i < filePathes.length; i++) {
                setValue(filePathesPtr + i * {{{ POINTER_SIZE }}}, filePathes[i], "i32");
            }

            callback(filePathesPtr, filePathes.length, futurePtr);
            
            stackRestore(sp);
            _siv3dMaybeAwake();
        };

        siv3dRegisterUserAction(function() {
            siv3dInputElement.click();
        });
    },
    $siv3dOpenDialogAsync__deps: [ "$siv3dInputElement", "$siv3dDialogFileReader", "$siv3dRegisterUserAction", "$FS", "siv3dMaybeAwake" ],
    siv3dOpenDialogAsync: function(filterStr, callback, futurePtr, acceptMuilitple) {
        const filter = UTF8ToString(filterStr);
        const callbackFn = {{{ makeDynCall('viii', 'callback') }}};
        siv3dOpenDialogAsync(filter, callbackFn, futurePtr, !!acceptMuilitple);
    },
    siv3dOpenDialogAsync__sig: "vii",
    siv3dOpenDialogAsync__deps: [ "$siv3dOpenDialogAsync" ],

    $siv3dSaveFileBuffer: null, 
    $siv3dSaveFileBufferWritePos: 0,
    $siv3dDefaultSaveFileName: null,

    siv3dDownloadFile: function(filePathPtr, fileNamePtr, mimeTypePtr) {
        const filePath = UTF8ToString(filePathPtr);
        const fileName = UTF8ToString(fileNamePtr);
        const mimeType = mimeTypePtr ? UTF8ToString(mimeTypePtr) : "application/octet-stream";
        const fileData = FS.readFile(filePath);

        const blob = new Blob([ fileData ], { type: mimeType });

        siv3dDownloadLink.href = URL.createObjectURL(blob);
        siv3dDownloadLink.download = fileName;

        siv3dRegisterUserAction(function() {
            siv3dDownloadLink.click();         
        });
    },
    siv3dDownloadFile__sig: "viii",
    siv3dDownloadFile__deps: [ "$siv3dRegisterUserAction" ],
});
mergeInto(LibraryManager.library, {
    $siv3dRegisterDragEnter: function(callback) {
        Module["canvas"]["ondragenter"] = function (e) {
            e.preventDefault();

            const types = e.dataTransfer.types;

            if (types.length > 0) {
                const adusted = siv3dAdjustPoint(e.pageX, e.pageY);
                callback(types[0] === 'Files' ? 1 : 0, adusted.x, adusted.y);
            }        
        };
    },
    siv3dRegisterDragEnter: function(ptr) {
        siv3dRegisterDragEnter({{{ makeDynCall('viii', 'ptr') }}});
    },
    siv3dRegisterDragEnter__deps: [ "$siv3dRegisterDragEnter" ],
    siv3dRegisterDragEnter__sig: "vi",

    $siv3dRegisterDragUpdate: function(callback) {
        Module["canvas"]["ondragover"] = function (e) {
            e.preventDefault();
            const adusted = siv3dAdjustPoint(e.pageX, e.pageY);
            callback(adusted.x, adusted.y);
        };
    },
    siv3dRegisterDragUpdate: function(ptr) {
        siv3dRegisterDragUpdate({{{ makeDynCall('vii', 'ptr') }}});
    },
    siv3dRegisterDragUpdate__sig: "vi",
    siv3dRegisterDragUpdate__deps: [ "$siv3dRegisterDragUpdate", "$siv3dAdjustPoint" ],

    $siv3dRegisterDragExit: function(callback) {
        Module["canvas"]["ondragexit"] = function (e) {
            e.preventDefault();
            callback();
        };
    },
    siv3dRegisterDragExit: function(ptr) {
        siv3dRegisterDragExit({{{ makeDynCall('v', 'ptr') }}});
    },
    siv3dRegisterDragExit__deps: [ "$siv3dRegisterDragExit" ],
    siv3dRegisterDragExit__sig: "vi",

    $siv3dDragDropFileReader: null,
    $siv3dRegisterDragDrop: function(callback) {
        Module["canvas"]["ondrop"] = function (e) {
            e.preventDefault();

            const items = e.dataTransfer.items;
            const adusted = siv3dAdjustPoint(e.pageX, e.pageY);

            if (items.length == 0) {
                return;
            }

            if (items[0].kind === 'string') {
                items[0].getAsString(function(str) {
                    const strPtr = allocate(intArrayFromString(str), ALLOC_NORMAL);
                    callback(strPtr, adusted.x, adusted.y);
                    Module["_free"](strPtr);
                })            
            } else if (items[0].kind === 'file') {
                const file = items[0].getAsFile();

                if (!siv3dDragDropFileReader) {
                    siv3dDragDropFileReader = new FileReader();
                }

                const filePath = "/tmp/" + file.name;

                siv3dDragDropFileReader.addEventListener("load", function onLoaded() {
                    FS.writeFile(filePath, new Uint8Array(siv3dDragDropFileReader.result));

                    const namePtr = allocate(intArrayFromString(filePath), ALLOC_NORMAL);
                    callback(namePtr, adusted.x, adusted.y);

                    siv3dDragDropFileReader.removeEventListener("load", onLoaded);
                });

                siv3dDragDropFileReader.readAsArrayBuffer(file);              
            }
        };
    },
    siv3dRegisterDragDrop: function(ptr) {
        siv3dRegisterDragDrop({{{ makeDynCall('viii', 'ptr') }}});
    },
    siv3dRegisterDragDrop__sig: "vi",
    siv3dRegisterDragDrop__deps: [ "$siv3dRegisterDragDrop", "$siv3dDragDropFileReader", "$FS" ],
});
mergeInto(LibraryManager.library, {
    siv3dGetJoystickInfo: function(joystickId) {
        return GLFW.joys[joystickId].id;
    },
    siv3dGetJoystickInfo__sig: "iiiii",

    glfwGetJoystickHats: function () {
        // Not supported.
        return 0;
    },
    glfwGetJoystickHats__sig: "iii",

    glfwGetKeysSiv3D: function (windowid) {
        const window = GLFW.WindowFromId(windowid);
        if (!window) return 0;
        if (!window.keysBuffer) {
            window.keysBuffer = Module["_malloc"](349 /* GLFW_KEY_LAST + 1 */)
            Module["HEAPU8"].fill(0, window.keysBuffer, window.keysBuffer + 348);
        }
        Module["HEAPU8"].set(window.keys, window.keysBuffer);
        return window.keysBuffer;
    },
    glfwGetKeysSiv3D__sig: "ii",
});
mergeInto(LibraryManager.library, { 
    glfwGetMonitorInfo_Siv3D: function(handle, displayID, xpos, ypos, w, h) {
        setValue(displayID, 1, 'i32');
        setValue(xpos, 0, 'i32');
        setValue(ypos, 0, 'i32');
        setValue(w, window.screen.width, 'i32');
        setValue(h, window.screen.height, 'i32');
    },
    glfwGetMonitorInfo_Siv3D__sig: "viiiiiiiiiii",
    glfwGetMonitorInfo_Siv3D__proxy: "sync",
    
    glfwGetMonitorWorkarea: function(handle, wx, wy, ww, wh) {
        setValue(wx, 0, 'i32');
        setValue(wy, 0, 'i32');
        setValue(ww, window.screen.availWidth, 'i32');
        setValue(wh, window.screen.availHeight, 'i32');
    },
    glfwGetMonitorWorkarea__sig: "viiiii",
    glfwGetMonitorWorkarea__proxy: "sync",

    glfwGetMonitorContentScale: function(handle, xscale, yscale) {
        setValue(xscale, 1, 'float');
        setValue(yscale, 1, 'float'); 
    },
    glfwGetMonitorContentScale__sig: "viii",
    glfwGetMonitorContentScale__proxy: "sync",

    glfwGetCursorPos__proxy: "sync",
    glfwCreateCursor__proxy: "sync",
    glfwSetCursorPos__proxy: "sync",
    glfwGetMonitors__proxy: "sync",
    glfwGetMonitorPhysicalSize__proxy: "sync",
    glfwGetVideoMode__proxy: "sync",
    glfwGetMonitorName__proxy: "sync",
    glfwMakeContextCurrent__proxy: "sync",
    glfwTerminate__proxy: "sync",
    glfwSetErrorCallback__proxy: "sync",
    glfwInit__proxy: "sync",
    glfwWindowHint__proxy: "sync",
    glfwCreateWindow__proxy: "sync",
    glfwSetWindowSizeLimits__proxy: "sync",
    glfwSetWindowUserPointer__proxy: "sync",
    glfwSetWindowPosCallback__proxy: "sync",
    glfwSetWindowSizeCallback__proxy: "sync",
    glfwSetFramebufferSizeCallback__proxy: "sync",
    glfwSetWindowIconifyCallback__proxy: "sync",
    glfwSetWindowFocusCallback__proxy: "sync",
    glfwPollEvents__proxy: "sync",
    glfwWindowShouldClose__proxy: "sync",
    glfwSetWindowShouldClose__proxy: "sync",
    glfwSetWindowTitle__proxy: "sync",
    glfwSetWindowPos__proxy: "sync",
    glfwMaximizeWindow__proxy: "sync",
    glfwRestoreWindow__proxy: "sync",
    glfwIconifyWindow__proxy: "sync",
    glfwSetWindowSize__proxy: "sync",
    glfwSetWindowSizeLimits__proxy: "sync",
    glfwGetFramebufferSize__proxy: "sync",
    glfwGetWindowPos__proxy: "sync",
    glfwGetWindowSize__proxy: "sync",
    glfwGetWindowAttrib__proxy: "sync",
    glfwGetWindowUserPointer__proxy: "sync",
});mergeInto(LibraryManager.library, {
    $siv3dXMLHTTPRequestList: [],
    $siv3dXMLHTTPRequestListNextID: 0,

    siv3dCreateXMLHTTPRequest: function() {
        const id = siv3dXMLHTTPRequestListNextID++;
        siv3dXMLHTTPRequestList[id] = new XMLHttpRequest();
        return id;
    },
    siv3dCreateXMLHTTPRequest__sig: "vi",
    siv3dCreateXMLHTTPRequest__deps: [ "$siv3dXMLHTTPRequestList", "$siv3dXMLHTTPRequestListNextID" ],

    siv3dSetXMLHTTPRequestWriteBackFile: function(id, fileNamePtr) {
        const http = siv3dXMLHTTPRequestList[id];
        const _file = UTF8ToString(fileNamePtr);

        http.addEventListener("load", function() {
            const index = _file.lastIndexOf('/');
            const destinationDirectory = PATH.dirname(_file);
            
            if (http.status >= 200 && http.status < 300) {
                // if a file exists there, we overwrite it
                try {
                    FS.unlink(_file);
                } catch (e) {}
                // if the destination directory does not yet exist, create it
                FS.mkdirTree(destinationDirectory);
                FS.createDataFile( _file.substr(0, index), _file.substr(index + 1), new Uint8Array(/** @type{ArrayBuffer}*/(http.response)), true, true, false);
            }
        });
    },
    siv3dSetXMLHTTPRequestWriteBackFile__sig: "vii",
    siv3dSetXMLHTTPRequestWriteBackFile__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dSetXMLHTTPRequestCallback: function(id, fnPtr, userDataPtr) {
        siv3dXMLHTTPRequestList[id].addEventListener("load", function() {
            {{{ makeDynCall("vii", "fnPtr") }}}(id, userDataPtr);
        });
    },
    siv3dSetXMLHTTPRequestCallback__sig: "viii",
    siv3dSetXMLHTTPRequestCallback__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dSetXMLHTTPRequestErrorCallback: function(id, fnPtr, userDataPtr) {
        siv3dXMLHTTPRequestList[id].addEventListener("error", function() {
            {{{ makeDynCall("vii", "fnPtr") }}}(id, userDataPtr);
        });
    },
    siv3dSetXMLHTTPRequestErrorCallback__sig: "viii",
    siv3dSetXMLHTTPRequestErrorCallback__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dSetXMLHTTPRequestProgressCallback: function(id, fnPtr, userDataPtr) {
        siv3dXMLHTTPRequestList[id].addEventListener("progress", function(e) {
            {{{ makeDynCall("viiii", "fnPtr") }}}(id, userDataPtr, e.total, e.loaded);
        });
    },
    siv3dSetXMLHTTPRequestProgressCallback__sig: "viii",
    siv3dSetXMLHTTPRequestProgressCallback__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dSetXMLHTTPRequestRequestHeader: function(id, namePtr, dataPtr) {
        const name = UTF8ToString(namePtr);
        const data = UTF8ToString(dataPtr);
        siv3dXMLHTTPRequestList[id].setRequestHeader(name, data);
    },
    siv3dSetXMLHTTPRequestRequestHeader__sig: "viii",
    siv3dSetXMLHTTPRequestRequestHeader__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dGetXMLHTTPRequestResponseHeaders: function(id) {
        const http = siv3dXMLHTTPRequestList[id];
        const responseHeaders = http.getAllResponseHeaders();
        return allocate(intArrayFromString(`HTTP/1.1 ${http.status} ${http.statusText}\r\n${responseHeaders}`), ALLOC_NORMAL);
    },
    siv3dGetXMLHTTPRequestResponseHeaders__sig: "ii",
    siv3dGetXMLHTTPRequestResponseHeaders__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dSendXMLHTTPRequest: function(id, dataPtr, dataSize) {
        {{{ runtimeKeepalivePush() }}}

        siv3dXMLHTTPRequestList[id].addEventListener("load", function() {
            {{{ runtimeKeepalivePop() }}}
        });
        siv3dXMLHTTPRequestList[id].addEventListener("error", function() {
            {{{ runtimeKeepalivePop() }}}
        });

        const data = dataPtr ? new Uint8Array(HEAPU8.buffer, dataPtr, dataSize) : null;
        siv3dXMLHTTPRequestList[id].send(data);
    },
    siv3dSendXMLHTTPRequest__sig: "viii",
    siv3dSendXMLHTTPRequest__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dOpenXMLHTTPRequest: function(id, methodPtr, urlPtr) {
        const http = siv3dXMLHTTPRequestList[id];
        const method = UTF8ToString(methodPtr);
        const url = UTF8ToString(urlPtr);
        
        http.open(method, url, true);
        http.responseType = "arraybuffer";
    },
    siv3dOpenXMLHTTPRequest__sig: "viii",
    siv3dOpenXMLHTTPRequest__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dAbortXMLHTTPRequest: function(id) {
        siv3dXMLHTTPRequestList[id].abort();
    },
    siv3dAbortXMLHTTPRequest__sig: "vi",
    siv3dAbortXMLHTTPRequest__deps: [ "$siv3dXMLHTTPRequestList" ],

    siv3dDeleteXMLHTTPRequest: function(id, methodPtr, urlPtr) {
        delete siv3dXMLHTTPRequestList[id];
    },
    siv3dDeleteXMLHTTPRequest__sig: "viii",
    siv3dDeleteXMLHTTPRequest__deps: [ "$siv3dXMLHTTPRequestList" ],
});
mergeInto(LibraryManager.library, {
    $siv3dDecodeCanvas: null,
    $siv3dDecodeCanvasContext: null,

    $siv3dDecodeImageFromFileImpl: function(wakeUp, src, size, data) {
        if (!siv3dDecodeCanvas) {
            siv3dDecodeCanvas = document.createElement('canvas');
            siv3dDecodeCanvasContext = siv3dDecodeCanvas.getContext("2d");
        }

        const imageData = new Uint8ClampedArray(HEAPU8.buffer, src, size);
        const imageBlob = new Blob([ imageData ]);
        const image = new Image();

        image.onload = function() {
            siv3dDecodeCanvas.width = image.width;
            siv3dDecodeCanvas.height = image.height;
            siv3dDecodeCanvasContext.drawImage(image, 0, 0);

            const decodedImageData = siv3dDecodeCanvasContext.getImageData(0, 0, image.width, image.height).data;
            const dataBuffer = Module["_malloc"](decodedImageData.length);

            HEAPU8.set(decodedImageData, dataBuffer);

            HEAPU32[(data>>2)+0] = dataBuffer;
            HEAPU32[(data>>2)+1] = decodedImageData.length;
            HEAPU32[(data>>2)+2] = image.width;
            HEAPU32[(data>>2)+3] = image.height;

            URL.revokeObjectURL(image.src);
            wakeUp();
        };
        image.onerror = function() {
            URL.revokeObjectURL(image.src);
            wakeUp();
        };
        image.src = URL.createObjectURL(imageBlob);
    },
    $siv3dDecodeImageFromFileImpl__deps: [ "$siv3dDecodeCanvas", "$siv3dDecodeCanvasContext" ],

#if ASYNCIFY
    siv3dDecodeImageFromFile: function(src, size, data) {
        return Asyncify.handleSleep(function (wakeUp) {
            siv3dDecodeImageFromFileImpl(wakeUp, src, size, data);
        });
    },
    siv3dDecodeImageFromFile__sig: "viii",
    siv3dDecodeImageFromFile__deps: ["$siv3dDecodeImageFromFileImpl", "$Asyncify"],
#elif PROXY_TO_PTHREAD
    siv3dDecodeImageFromFile: function(ctx, src, size, data) {
        siv3dDecodeImageFromFileImpl(function () {
            Module["_emscripten_proxy_finish"](ctx);
        }, src, size, data);
    },
    siv3dDecodeImageFromFile__sig: "viiii",
    siv3dDecodeImageFromFile__deps: ["$siv3dDecodeImageFromFileImpl"],
#else
    siv3dDecodeImageFromFile: function(_, _, arg) {
        HEAP32[(arg>>2)+0] = 0;
        HEAP32[(arg>>2)+1] = 0;
        HEAPU32[(arg>>2)+2] = 0;
        HEAPU32[(arg>>2)+3] = 0;
    },
    siv3dDecodeImageFromFile__sig: "viii",
#endif
});
mergeInto(LibraryManager.library, {
    $siv3dAllowedKeyBindings: [],

    siv3dAddAllowedKeyBinding: function(keyCode, ctrlKey, shiftKey, altKey, metaKey, allowed) {
        const key = {
            keyCode,
            ctrlKey: !!ctrlKey, shiftKey: !!shiftKey, altKey: !!altKey, metaKey: !!metaKey
        };

        function compareObject(obj) {
            return JSON.stringify(obj) == JSON.stringify(key);
        }

        const index = siv3dAllowedKeyBindings.findIndex(compareObject);

        if (allowed) {
            if (index === -1) {
                siv3dAllowedKeyBindings.push(key);
            }
        } else {
            if (index !== -1) {
                delete siv3dAllowedKeyBindings[index];
            }
        }
    },
    siv3dAddAllowedKeyBinding__sig: "viiiiii",
    siv3dAddAllowedKeyBinding__deps: [ "$siv3dAllowedKeyBindings" ],

    siv3dDisableAllKeyBindings: function(disabled) {
        function onKeyEvent(e) {
            if (siv3dGetTextInputFocused()) {
                return;
            }

            const key = {
                keyCode: GLFW.DOMToGLFWKeyCode(e.keyCode),
                ctrlKey: e.ctrlKey, shiftKey: e.shiftKey, altKey: e.altKey, metaKey: e.metaKey
            };

            function compareObject(obj) {
                return JSON.stringify(obj) == JSON.stringify(key);
            }
    
            const index = siv3dAllowedKeyBindings.findIndex(compareObject);

            if (index === -1) {
                e.preventDefault();
            }
        }

        if (disabled) {
            window.addEventListener("keydown", onKeyEvent);
        } else {
            window.removeEventListener("keydown", onKeyEvent);
        }
    },
    siv3dDisableAllKeyBindings__sig: "vi",
    siv3dDisableAllKeyBindings__deps: [ "$siv3dAllowedKeyBindings", "$siv3dGetTextInputFocused" ],
});
mergeInto(LibraryManager.library, { 
    $siv3dActiveTouches: [],

    $siv3dAdjustPoint: function (x, y) {
        const rect = Module["canvas"].getBoundingClientRect();
        const cw = Module["canvas"].width;
        const ch = Module["canvas"].height;

        const scrollX = ((typeof window.scrollX !== 'undefined') ? window.scrollX : window.pageXOffset);
        const scrollY = ((typeof window.scrollY !== 'undefined') ? window.scrollY : window.pageYOffset);

        let adjustedX = x - (scrollX + rect.left);
        let adjustedY = y - (scrollY + rect.top);

        adjustedX = adjustedX * (cw / rect.width);
        adjustedY = adjustedY * (ch / rect.height);

        return { x: adjustedX, y: adjustedY };
    },
    
    $siv3dOnTouchStart: function(e) {
        siv3dActiveTouches = Array.from(e.touches);
        // e.preventDefault()
    },

    $siv3dOnTouchEnd: function(e) {
        siv3dActiveTouches = Array.from(e.touches);
        // e.stopPropagation();
    },

    $siv3dOnTouchMove: function(e) {
        siv3dActiveTouches = Array.from(e.touches);
        // e.stopPropagation();
    },

    siv3dRegisterTouchCallback: function() {
        Module["canvas"].addEventListener("touchstart", siv3dOnTouchStart);
        Module["canvas"].addEventListener("touchmove", siv3dOnTouchMove);
    },
    siv3dRegisterTouchCallback__sig: "v",
    siv3dRegisterTouchCallback__deps: [ "$siv3dOnTouchMove", "$siv3dOnTouchStart", "$siv3dActiveTouches" ],

    siv3dUnregisterTouchCallback: function() {
        Module["canvas"].removeEventListener("touchstart", siv3dOnTouchStart);
        Module["canvas"].removeEventListener("touchmove", siv3dOnTouchMove);
    },
    siv3dUnregisterTouchCallback__sig: "v",
    siv3dUnregisterTouchCallback__deps: [ "$siv3dOnTouchMove", "$siv3dOnTouchStart" ],

    siv3dGetPrimaryTouchPoint: function(pX, pY) {
        if (siv3dActiveTouches.length > 0) {
            const touch = siv3dActiveTouches[0];
            const adjusted = siv3dAdjustPoint(touch.pageX, touch.pageY);
            
            setValue(pX, adjusted.x, 'double');
            setValue(pY, adjusted.y, 'double');
            return 1;
        } else {
            return 0;
        }
    },
    siv3dGetPrimaryTouchPoint__sig: "iii",
    siv3dGetPrimaryTouchPoint__deps: [ "$siv3dActiveTouches", "$siv3dAdjustPoint" ],
});
mergeInto(LibraryManager.library, {
    $siv3dNotifications: [],

    siv3dRequestNotificationPermission: function(callback, callbackArg) {
        if (Notification.permission === "granted") {
            {{{ makeDynCall('vii', 'callback') }}}(1 /* NotificationPermission.Granted */, callbackArg);
            _siv3dMaybeAwake();
        } else {
            siv3dRegisterUserAction(function () {
                Notification.requestPermission().then(function(v) {
                    if (v === "granted") {
                        {{{ makeDynCall('vii', 'callback') }}}(1 /* NotificationPermission.Granted */, callbackArg);
                    } else {
                        {{{ makeDynCall('vii', 'callback') }}}(2 /* NotificationPermission.Denied */, callbackArg);
                    }
                    _siv3dMaybeAwake();
                });
            });
        }
    },
    siv3dRequestNotificationPermission__sig: "vii",
    siv3dRequestNotificationPermission__deps: [ "siv3dMaybeAwake"],

    siv3dCreateNotification: function(title, body, actionsNum, actionTexts, callback, callbackArg) {
        if (!window.Notification && Notification.permission !== "granted") {
            {{{ makeDynCall('vii', 'callback') }}}(0, callbackArg);
            _siv3dMaybeAwake();
            return 0;
        }

        const idx = GL.getNewId(siv3dNotifications);

        const titleText = UTF8ToString(title);
        const bodyText = UTF8ToString(body);
        let actions = [];

        for (var i = 0; i < actionsNum; i++) {
            const textPtr = getValue(actionTexts + i * 4, "i32");
            const actionText = UTF8ToString(textPtr);

            actions.push({ title: actionText, action: actionText });
        }

        siv3dNotifications[idx] = new Notification(titleText, { body: bodyText, actions: actions });
        {{{ makeDynCall('vii', 'callback') }}}(idx, callbackArg);
        _siv3dMaybeAwake();

        return idx;
    },
    siv3dCreateNotification__sig: "iiiiiii",
    siv3dCreateNotification__deps: [ "$siv3dRegisterUserAction", "$siv3dNotifications", "siv3dMaybeAwake" ],

    siv3dRegisterNotificationCallback: function(id, callback, callbackArg) {
        const notificattion = siv3dNotifications[id];

        notificattion.onclick = function() {
            {{{ makeDynCall('viii', 'callback') }}}(id, 1 /* ToastNotificationState.Activated */, callbackArg);
            _siv3dMaybeAwake();
        }
        notificattion.onshow = function() {
            {{{ makeDynCall('viii', 'callback') }}}(id, 2 /* ToastNotificationState.Shown */, callbackArg);
            _siv3dMaybeAwake();
        }
        notificattion.onclose = function() {
            {{{ makeDynCall('viii', 'callback') }}}(id, 5 /* ToastNotificationState.TimedOut */, callbackArg);
            _siv3dMaybeAwake();
        }
        notificattion.onerror = function() {
            {{{ makeDynCall('viii', 'callback') }}}(id, 6 /* ToastNotificationState.Error */, callbackArg);
            _siv3dMaybeAwake();
        }
    },
    siv3dRegisterNotificationCallback__sig: "viii",
    siv3dRegisterNotificationCallback__deps: [ "$siv3dNotifications", "siv3dMaybeAwake" ],

    siv3dCloseNotification: function(id) {
        const notificattion = siv3dNotifications[id];
        notificattion.close();

        delete siv3dNotifications[id];
    },
    siv3dCloseNotification__sig: "vi",
    siv3dCloseNotification__deps: [ "$siv3dNotifications" ],

    siv3dQueryNotificationPermission: function() {
        const status = {
            "default": 0,
            "granted": 1,
            "denied": 2
        };
        return status[Notification.permission];
    },
    siv3dQueryNotificationPermission__sig: "iv",
});mergeInto(LibraryManager.library, {
    //
    // System
    //
    $siv3dSetThrowJSException: function(ex) {
        const text = allocate(intArrayFromString(ex), ALLOC_STACK);
        Module["_siv3dThrowException"](text);
    },

    siv3dCallOnAlert: function(textPtr) {
        const text = UTF8ToString(textPtr);
        Module["onAlert"] && Module["onAlert"](text);
    },
    siv3dCallOnAlert__sig: "vi",

    siv3dLocateFile: function() {
        if (Module["locateFile"]) {
            const origin = Module["locateFile"]("");
            return allocate(intArrayFromString(origin), ALLOC_NORMAL);
        } else {
            return 0;
        }
    },
    siv3dLocateFile__sig: "iv",

    siv3dSetCursorStyle: function(style) {
        const styleText = UTF8ToString(style);
        Module["canvas"]["style"]["cursor"] = styleText;
    },
    siv3dSetCursorStyle__sig: "vi",

    siv3dRequestFullscreen: function() {
        siv3dRegisterUserAction(function () {
            Browser.requestFullscreen();
        });
    },
    siv3dRequestFullscreen__sig: "v",
    siv3dRequestFullscreen__deps: [ "$siv3dRegisterUserAction", "$Browser" ],

    siv3dExitFullscreen: function() {
        siv3dRegisterUserAction(function () {
            Browser.exitFullscreen();
        });
    },
    siv3dExitFullscreen__sig: "v",
    siv3dExitFullscreen__deps: [ "$siv3dRegisterUserAction", "$Browser" ],

    //
    // MessageBox
    //
    siv3dShowMessageBox: function(messagePtr, type) {
        const message = UTF8ToString(messagePtr);

        if (type === 0) {
            /* MessageBoxButtons.OK */
            window.alert(message);
            return 0; /* MessageBoxResult.OK */
        } else if (type === 1) {
            /* MessageBoxButtons.OKCancel */
            return window.confirm(message) ? 0 /* MessageBoxResult.OK */ : 1 /* MessageBoxResult.Cancel */;
        }

        return 4; /* MessageBoxSelection.None */
    },
    siv3dShowMessageBox__sig: "iii",

    //
    // AngelScript Support
    //
    siv3dCallIndirect: function(funcPtr, funcTypes, retPtr, argsPtr) {
        let args = [];
        let funcTypeIndex = funcTypes;
        let argsPtrIndex = argsPtr;

        const retType = HEAPU8[funcTypeIndex++];

        while (true) {
            const funcType = HEAPU8[funcTypeIndex++];

            if (funcType === 0) break;

            switch (funcType) {
                case 105: // 'i':
                    args.push(HEAP32[argsPtrIndex >> 2]);
                    argsPtrIndex += 4;
                    break;
                case 102: // 'f':
                    args.push(HEAPF32[argsPtrIndex >> 2]);
                    argsPtrIndex += 4;
                    break;
                case 100: // 'd':
                    argsPtrIndex += (8 - argsPtrIndex % 8);
                    args.push(HEAPF64[argsPtrIndex >> 3]);
                    argsPtrIndex += 8;
                    break;
                default:
                    err("Unrecognized Function Type");
            }
        }

        const retValue = wasmTable.get(funcPtr).apply(null, args);

        switch (retType) {
            case 105: // 'i':
                HEAP32[retPtr >> 2] = retValue;
                break;
            case 102: // 'f':
                HEAPF32[retPtr >> 2] = retValue;
                break;
            case 100: // 'd':
                HEAPF64[retPtr >> 3] = retValue;
                break;
            case 118: // 'v':
                break;
            default:
                err("Unrecognized Function Type");
        }
    },
    siv3dCallIndirect__sig: "viiii",

    //
    // Misc
    //
    siv3dLaunchBrowser: function(url) {
        const urlString = UTF8ToString(url);
        
        siv3dRegisterUserAction(function () {
            window.open(urlString, '_blank')
        });
    },
    siv3dLaunchBrowser__sig: "vi",
    siv3dLaunchBrowser__deps: [ "$siv3dRegisterUserAction" ],

    siv3dGetURLParameters: function() {
        const params = new URL(document.location).searchParams.entries();
        const paramStrs = [];
        
        for (const param of params)
        {
            paramStrs.push(...param);
        }

        const dataPos = (Module["_malloc"](4 * (paramStrs.length + 1))) / 4;
        HEAPU32.fill(0, dataPos, dataPos + paramStrs.length + 1);

        for (var i = 0; i < paramStrs.length; i++)
        {
            HEAP32[dataPos + i] = allocate(intArrayFromString(paramStrs[i]), ALLOC_NORMAL);
        }

        return dataPos * 4; // dataPos * sizeof(uint32_t)
    },
    siv3dGetURLParameters__sig: "iv",

    //
    // Asyncify Support
    //
#if ASYNCIFY

    $siv3dAwakeFunction: null,
    siv3dSleepUntilWaked: function() {
        Asyncify.handleSleep(function(wakeUp) {
            siv3dAwakeFunction = wakeUp;
        });
        return 0;
    },
    siv3dSleepUntilWaked__sig: "iv",
    siv3dSleepUntilWaked__deps: [ "$Asyncify", "$siv3dAwakeFunction" ],

    siv3dMaybeAwake: function() {
        if (siv3dAwakeFunction) {
            let awake = siv3dAwakeFunction;
            siv3dAwakeFunction = null;

            callUserCallback(awake);
        }
    },
    siv3dMaybeAwake__sig: "v",
    siv3dMaybeAwake__deps: [ "$siv3dAwakeFunction" ],
    
    siv3dRequestAnimationFrame: function() {
        Asyncify.handleSleep(function(wakeUp) {
            requestAnimationFrame(function() {
                callUserCallback(wakeUp);
            });
        });
    },
    siv3dRequestAnimationFrame__sig: "v", 
    siv3dRequestAnimationFrame__deps: [ "$Asyncify", "$maybeExit", "abort" ],

#else
    siv3dSleepUntilWaked: function() {
        return -1;
    },
    siv3dSleepUntilWaked__sig: "iv",
    siv3dMaybeAwake: function() {
        // nop
    },
    siv3dMaybeAwake__sig: "v",
    siv3dRequestAnimationFrame: function() {
        // nop
    },
    siv3dRequestAnimationFrame__sig: "v",
#endif
});
mergeInto(LibraryManager.library, {
    $siv3dTextInputElement: null,
    $siv3dTextInputCompositionRange: null,

    siv3dInitTextInput: function() {
        const textInput = document.createElement("div");
        textInput.id = "textinput";
        textInput.contentEditable = "plaintext-only";
        textInput.style.position = "absolute";
        textInput.style.width = "100px";
        textInput.style.zIndex = -2;
        textInput.style.whiteSpace = "pre-wrap";
        textInput.autocomplete = false;

        const maskDiv = document.createElement("div");
        maskDiv.style.background = "white";
        maskDiv.style.position = "absolute";
        maskDiv.style.width = "100%";
        maskDiv.style.height = "100%";
        maskDiv.style.zIndex = -1;

        /**
         * @type { HTMLCanvasElement }
         */
        const canvas = Module["canvas"];

        canvas.parentNode.prepend(textInput);
        canvas.parentNode.prepend(maskDiv);

        siv3dTextInputElement = textInput;
    },
    siv3dInitTextInput__sig: "v",
    siv3dInitTextInput__proxy: "sync",
    siv3dInitTextInput__deps: [ "$siv3dTextInputElement" ],

    $siv3dRegisterTextInputCallback: function(callbackFn) {
        let composing = false;
        let insertCompositionTextInvoked = false;

        siv3dTextInputElement.addEventListener('input', function (e) {
            if (e.isComposing || composing) {
                return;
            }

            if (e.inputType == "insertText") {
                if (e.data) {
                    for (const char of e.data) {
                        const codePoint = char.codePointAt(0);
                        callbackFn(codePoint);
                    }
                }
            } else if (e.inputType == "insertFromPaste") {
                if (e.data) {
                    for (const char of e.data) {
                        const codePoint = char.codePointAt(0);
                        callbackFn(codePoint);
                    }
                } else {
                    navigator.clipboard.readText().then(
                        data => {
                            for (const char of data) {
                                const codePoint = char.codePointAt(0);
                                callbackFn(codePoint);
                            }
                        }
                    );
                }
            } else if (e.inputType == "deleteContentBackward") {
                callbackFn(8);
            } else if (e.inputType == "deleteContentForward") {
                callbackFn(0x7F);
            }
        });
        siv3dTextInputElement.addEventListener('beforeinput', function (e) {
            if (e.inputType == "insertCompositionText" || (composing && e.inputType == "insertText")) {
                siv3dTextInputCompositionRange = e.getTargetRanges()[0];

                if (!insertCompositionTextInvoked && !!siv3dTextInputCompositionRange) {
                    const length = siv3dTextInputCompositionRange.endOffset - siv3dTextInputCompositionRange.startOffset;
                    for (var i = 0; i < length; i++) {
                        callbackFn(8);
                    }
                }
                insertCompositionTextInvoked = true;
            }
        });
        siv3dTextInputElement.addEventListener('compositionstart', function (e) {
            composing = true;
            insertCompositionTextInvoked = false;
        });
        siv3dTextInputElement.addEventListener('compositionend', function (e) {
            composing = false;
            siv3dTextInputCompositionRange = null;
            for (const char of e.data) {
                const codePoint = char.codePointAt(0);
                callbackFn(codePoint);
            }
        });
    },

    siv3dRegisterTextInputCallback: function(callback) {
        siv3dRegisterTextInputCallback({{{ makeDynCall("vi", "callback") }}});
    },
    siv3dRegisterTextInputCallback__sig: "vi",
    siv3dRegisterTextInputCallback__proxy: "sync",
    siv3dRegisterTextInputCallback__deps: [ "$siv3dTextInputElement", "$siv3dRegisterTextInputCallback", "$siv3dTextInputCompositionRange" ],

    siv3dGetTextInputCompositionRange: function(start, end) {
        if (siv3dTextInputCompositionRange) {
            setValue(start, siv3dTextInputCompositionRange.startOffset, 'i32');
            setValue(end, siv3dTextInputCompositionRange.endOffset, 'i32');
        } else {
            setValue(start, 0, 'i32');
            setValue(end, 0, 'i32');
        }
    },
    siv3dGetTextInputCompositionRange__sig: "vii",
    siv3dGetTextInputCompositionRange__proxy: "sync",
    siv3dGetTextInputCompositionRange__deps: [ "$siv3dTextInputCompositionRange" ],

    siv3dRegisterTextInputMarkedCallback: function(callback) {
        siv3dTextInputElement.addEventListener('compositionupdate', function (e) {
            const strPtr = allocate(intArrayFromString(e.data), ALLOC_NORMAL);
            {{{ makeDynCall('vi', 'callback') }}}(strPtr);
            Module["_free"](strPtr);
        })
        siv3dTextInputElement.addEventListener('compositionend', function (e) {
            {{{ makeDynCall('vi', 'callback') }}}(0);
        });
    },
    siv3dRegisterTextInputMarkedCallback__sig: "vi",
    siv3dRegisterTextInputMarkedCallback__proxy: "sync",
    siv3dRegisterTextInputMarkedCallback__deps: [ "$siv3dTextInputElement" ],

    siv3dRequestTextInputFocus: function(isFocusRequired) {
        const isFocusRequiredBool = isFocusRequired != 0;

        if (isFocusRequiredBool) {
            if (document.activeElement != siv3dTextInputElement) {
                siv3dRegisterUserAction(function () {
                    siv3dTextInputElement.focus();
                });
            }
        } else {
            if (document.activeElement == siv3dTextInputElement) {
                siv3dRegisterUserAction(function () {
                    siv3dTextInputElement.blur();
                });
            }
        }
    },
    siv3dRequestTextInputFocus__sig: "vi",
    siv3dRequestTextInputFocus__proxy: "sync",
    siv3dRequestTextInputFocus__deps: [ "$siv3dRegisterUserAction", "$siv3dTextInputElement" ],

    siv3dSetTextInputText: function(ptr) {
        /** @type { string } */
        const newText = UTF8ToString(ptr);
        siv3dTextInputElement.textContent = " ".repeat([...newText].length);
    },
    siv3dSetTextInputText__sig: "vi",
    siv3dSetTextInputText__proxy: "sync",
    siv3dSetTextInputText__deps: [ "$siv3dTextInputElement" ],

    siv3dSetTextInputCursor: function(index) {
        const targetTextNode = siv3dTextInputElement.childNodes[0];

        if (!targetTextNode) {
            return;
        }

        const selection = window.getSelection();
        const range = document.createRange();

        range.selectNode(targetTextNode);
        range.setStart(targetTextNode, index);
        range.setEnd(targetTextNode, index);

        selection.removeAllRanges();
        selection.addRange(range);
    },
    siv3dSetTextInputCursor__sig: "vi",
    siv3dSetTextInputCursor__proxy: "proxy",
    siv3dSetTextInputCursor__deps: [ "$siv3dTextInputElement" ],

    siv3dGetTextInputCursor: function() {
        const selection = window.getSelection();
        const targetTextNode = siv3dTextInputElement.childNodes[0];

        if (selection.focusNode == targetTextNode) {
            return selection.focusOffset;
        } else {
            return 0;
        }
    },
    siv3dGetTextInputCursor__sig: "iv",
    siv3dGetTextInputCursor__proxy: "sync",
    siv3dGetTextInputCursor__deps: [ "$siv3dTextInputElement" ],

    $siv3dGetTextInputFocused: function() {
        return document.activeElement == siv3dTextInputElement;
    },
    $siv3dGetTextInputFocused__deps: [ "$siv3dTextInputElement" ],
});
mergeInto(LibraryManager.library, {
    $siv3dTextRenderingCanvas: null,
    $siv3dTextRenderingCanvasContext: null,

    siv3dRenderText: function(utf32CodePoint, data) {
        if (!siv3dTextRenderingCanvas) {
            siv3dTextRenderingCanvas = document.createElement('canvas');
            siv3dTextRenderingCanvasContext = siv3dTextRenderingCanvas.getContext("2d");
        }

        const text = String.fromCodePoint(utf32CodePoint);
        const fontSize = 64;
        const fontName = "sans-serif";

        siv3dTextRenderingCanvasContext.fillStyle = "0x0";
        siv3dTextRenderingCanvasContext.font = `${fontSize}px '${fontName}'`;
        siv3dTextRenderingCanvasContext.textBaseline = "bottom";

        const textMetrix = siv3dTextRenderingCanvasContext.measureText(text);
        const fontWidth = Math.ceil(Math.abs(textMetrix.actualBoundingBoxLeft) + Math.abs(textMetrix.actualBoundingBoxRight)) || 1;
        const fontHeight = Math.ceil(Math.abs(textMetrix.actualBoundingBoxAscent) + Math.abs(textMetrix.actualBoundingBoxDescent)) || 1;
        const fontXAdvance = textMetrix.width;

        siv3dTextRenderingCanvasContext.clearRect(0, 0, siv3dTextRenderingCanvas.width, siv3dTextRenderingCanvas.height);
        siv3dTextRenderingCanvasContext.fillText(text, Math.abs(textMetrix.actualBoundingBoxLeft), fontHeight);

        const textBitmap = siv3dTextRenderingCanvasContext.getImageData(0, 0, fontWidth, fontHeight).data;
        const dataBuffer = Module["_malloc"](textBitmap.length);

        HEAPU8.set(textBitmap, dataBuffer);

        HEAPU32[data>>2] = utf32CodePoint; data += 4;                       // glyphIndex
        HEAP32[data>>2] = dataBuffer; data += 4;                            // buffer
        HEAP16[data>>1] = textMetrix.actualBoundingBoxLeft; data += 2;      // left
        HEAP16[data>>1] = fontHeight; data += 2;                            // top
        HEAP16[data>>1] = fontWidth; data += 2;                             // width
        HEAP16[data>>1] = fontHeight; data += 2;                            // height
        HEAP16[data>>1] = fontSize; data += 2;                              // ascender
        HEAP16[data>>1] = 0; data += 2;                                     // descender
        data += 4;                                                          // padding
        HEAPF64[data>>3] = fontXAdvance; data += 8;                         // xAdvance
        HEAPF64[data>>3] = 0; data += 8;                                    // yAdvance
    },
    siv3dRenderText__sig: "vii",
    siv3dRenderText__deps: [ "$siv3dTextRenderingCanvas", "$siv3dTextRenderingCanvasContext" ],
});
mergeInto(LibraryManager.library, {
    siv3dRegisterTextToSpeechLanguagesUpdateHander: function(callback, callbackArg) {
        window.speechSynthesis.onvoiceschanged = function() {
            {{{ makeDynCall('vi', 'callback') }}}(callbackArg);
        };
    },
    siv3dRegisterTextToSpeechLanguagesUpdateHander__sig: "vii",

    siv3dEnumerateAvailableTextToSpeechLanguages: function(returnPtr) {
        const LanguageNameToLanguageCodeList = {
            "ar-SA": 1025,
            "zh-CN": 2052,
            "zh-HK": 3076,
            "zh-TW": 1028,
            "en-AU": 3081,
            "en-GB": 2057,
            "en-US": 1033,
            "fr-FR": 1036,
            "de-DE": 1031,
            "hi-IN": 1081,
            "it-IT": 1040,
            "ja-JP": 1041,
            "ko-KR": 1042,
            "pt-BR": 1046,
            "ru-RU": 1049,
            "es-ES": 1034
        };
        
        const voices = window.speechSynthesis.getVoices();
        let listBufferPtr = Module["_malloc"](voices.length * 4 * 2);

        setValue(returnPtr, voices.length, "i32");
        setValue(returnPtr + 4, listBufferPtr, "i32");

        for(var i = 0; i < voices.length; i++) {
            const languageCode = LanguageNameToLanguageCodeList[voices[i].lang];
             
            setValue(listBufferPtr + 0, languageCode, "i32");
            setValue(listBufferPtr + 4, voices[i].default, "i32");

            listBufferPtr += 8;
        }
    },
    siv3dEnumerateAvailableTextToSpeechLanguages__sig: "vi",

    siv3dStartTextToSpeechLanguages: function(textPtr, rate, volume, languageCode) {
        const LanguageCodeToLanguageNameList = {
            1025: "ar-SA",
            2052: "zh-CN",
            3076: "zh-HK",
            1028: "zh-TW",
            3081: "en-AU",
            2057: "en-GB",
            1033: "en-US",
            1036: "fr-FR",
            1031: "de-DE",
            1081: "hi-IN",
            1040: "it-IT",
            1041: "ja-JP",
            1042: "ko-KR",
            1046: "pt-BR",
            1049: "ru-RU",
            1034: "es-ES"
        };
        const text = UTF8ToString(textPtr);

        const speechUtter = new SpeechSynthesisUtterance(text);

        speechUtter.lang = LanguageCodeToLanguageNameList[languageCode];
        speechUtter.rate = rate;
        speechUtter.volume = volume;

        window.speechSynthesis.speak(speechUtter);
    },
    siv3dStartTextToSpeechLanguages__sig: "viiii",

    siv3dIsSpeakingTextToSpeechLanguages: function() {
        return window.speechSynthesis.speaking;
    },
    siv3dIsSpeakingTextToSpeechLanguages__sig: "iv",

    siv3dPauseTextToSpeechLanguages: function() {
        window.speechSynthesis.pause();
    },
    siv3dPauseTextToSpeechLanguages__sig: "v",

    siv3dResumeTextToSpeechLanguages: function() {
        window.speechSynthesis.resume();
    },
    siv3dResumeTextToSpeechLanguages__sig: "v",
});
mergeInto(LibraryManager.library, {
    $siv3dHasUserActionTriggered: false,
    $siv3dPendingUserActions: [],

    $siv3dTriggerUserAction: function() {
        for (var i = 0; i < siv3dPendingUserActions.length; i++) {
            (siv3dPendingUserActions[i])();
        }

        siv3dPendingUserActions.splice(0);
        siv3dHasUserActionTriggered = false;
    },
    $siv3dTriggerUserAction__deps: [ "$siv3dPendingUserActions" ],

    $siv3dRegisterUserAction: function(func) {
        siv3dPendingUserActions.push(func);
    },
    $siv3dRegisterUserAction__deps: [ "$siv3dPendingUserActions", "$autoResumeAudioContext", "$dynCall" ],

    $siv3dUserActionHookCallBack: function() {
        if (!siv3dHasUserActionTriggered) {
            setTimeout(siv3dTriggerUserAction, 30);
            siv3dHasUserActionTriggered = true;
        }
    },
    $siv3dUserActionHookCallBack__deps: [ "$siv3dHasUserActionTriggered", "$siv3dTriggerUserAction" ],

    $siv3dUserActionTouchEndCallBack: function(e) {
        siv3dTriggerUserAction();
        e.preventDefault();
    },
    $siv3dUserActionHookCallBack__deps: [ "$siv3dHasUserActionTriggered", "$siv3dTriggerUserAction" ],

    siv3dStartUserActionHook: function() {
        Module["canvas"].addEventListener('touchend', siv3dUserActionTouchEndCallBack);
        Module["canvas"].addEventListener('mousedown', siv3dUserActionHookCallBack);
        window.addEventListener('keydown', siv3dUserActionHookCallBack);
    },
    siv3dStartUserActionHook__sig: "v",
    siv3dStartUserActionHook__deps: [ "$siv3dUserActionHookCallBack", "$siv3dUserActionTouchEndCallBack", "$siv3dHasUserActionTriggered" ],

    siv3dStopUserActionHook: function() {
        Module["canvas"].removeEventListener('touchend', siv3dUserActionTouchEndCallBack);
        Module["canvas"].removeEventListener('mousedown', siv3dUserActionHookCallBack);
        window.removeEventListener('keydown', siv3dUserActionHookCallBack);
    },
    siv3dStopUserActionHook__sig: "v",
    siv3dStopUserActionHook__deps: [ "$siv3dUserActionHookCallBack", "$siv3dUserActionTouchEndCallBack" ],

});
mergeInto(LibraryManager.library, {
    $videoElements: [],

    $siv3dOpenVideoStream: function(filename, callback, callbackArg) {
        const videoData = FS.readFile(UTF8ToString(fileName));
        const media_source = new MediaSource();
       
        const video = document.createElement("video");
        video["muted"] = true;
        video["autoplay"] = true;
        video["playsInline"] = true;

        media_source.addEventListener('sourceopen', function() {
            const source_buffer = media_source.addSourceBuffer('video/mp4');
			source_buffer.addEventListener("updateend", function () {
                media_source.endOfStream()
            });
			source_buffer.appendBuffer(videoData)
        });

        video.addEventListener('loadedmetadata', function onLoaded() {
            const idx = GL.getNewId(videoElements);

            video.removeEventListener('loadedmetadata', onLoaded);
            videoElements[idx] = video;

            if (callback) {{{ makeDynCall('vii', 'callback') }}}(idx, callbackArg);
            _siv3dMaybeAwake();
        });

        video.src = URL.createObjectURL(media_source);
    },
    $siv3dOpenVideoStream__deps: [ "siv3dMaybeAwake" ],

    siv3dOpenVideo: function(fileName, callback, callbackArg) {
        const videoData = FS.readFile(UTF8ToString(fileName));
        const videoBlob = new Blob([ videoData ], { type: "video/mp4" });
       
        const video = document.createElement("video");
        video["muted"] = true;
        video["autoplay"] = true;
        video["playsInline"] = true;

        video.addEventListener('loadedmetadata', function onLoaded() {
            const idx = GL.getNewId(videoElements);

            video.removeEventListener('loadedmetadata', onLoaded);
            videoElements[idx] = video;

            if (callback) {{{ makeDynCall('vii', 'callback') }}}(idx, callbackArg);
            _siv3dMaybeAwake();
        });

        video.src = URL.createObjectURL(videoBlob);
    },
    siv3dOpenVideo__sig: "viii",
    siv3dOpenVideo__deps: [ "$FS", "$videoElements", "$siv3dRegisterUserAction", "siv3dMaybeAwake" ],

    siv3dOpenCamera: function(width, height, callback, callbackArg) {
        const constraint = {
            video: { 
                width : width > 0 ? width : undefined, 
                height: height > 0 ? height : undefined 
            },
            audio: false
        };

        navigator.mediaDevices.getUserMedia(constraint).then(
            function(stream) {
                const video = document.createElement("video");

                video["playsInline"] = true;          
                video.addEventListener('loadedmetadata', function onLoaded() {
                    const idx = GL.getNewId(videoElements);

                    video.removeEventListener('loadedmetadata', onLoaded);
                    videoElements[idx] = video;

                    if (callback) {{{ makeDynCall('vii', 'callback') }}}(idx, callbackArg);
                    _siv3dMaybeAwake();
                });

                video.srcObject = stream;
            }
        ).catch(function(_) {
            if (callback) {{{ makeDynCall('vii', 'callback') }}}(0, callbackArg);
            _siv3dMaybeAwake();
        })
    },
    siv3dOpenCamera__sig: "viiii",
    siv3dOpenCamera__deps: [ "$videoElements", "siv3dMaybeAwake" ],

    siv3dSetCameraResolution: function(idx, width, height, callback, callbackArg) {
        /** @type { HTMLVideoElement } */
        const video = videoElements[idx];
        /** @type { MediaStreamTrack } */
        const stream = video.srcObject.getVideoTracks()[0];

        const constraint = {
            video: { width, height },
            audio: false
        };

        stream.applyConstraints(constraint).then(
            function () {
                if (callback) {{{ makeDynCall('vii', 'callback') }}}(idx, callbackArg);
                _siv3dMaybeAwake();
            }
        );
    },
    siv3dSetCameraResolution__sig: "viiiii",
    siv3dSetCameraResolution__deps: [ "$videoElements", "siv3dMaybeAwake" ],

    siv3dQueryCameraAvailability: function () {
        return !!navigator.getUserMedia;
    },
    siv3dQueryCameraAvailability__sig: "iv",

    siv3dRegisterVideoTimeUpdateCallback: function(idx, callback, callbackArg) {
        const video = videoElements[idx];

        if (callback) {
            video.ontimeupdate = function() {
                {{{ makeDynCall('vi', 'callback') }}}(callbackArg);
                _siv3dMaybeAwake();
            }
        } else {
            video.ontimeupdate = null;
        }
    },
    siv3dRegisterVideoTimeUpdateCallback__sig: "viii",
    siv3dRegisterVideoTimeUpdateCallback__deps: [ "$videoElements", "siv3dMaybeAwake" ], 

    siv3dCaptureVideoFrame: function(target, level, internalFormat, width, height, border, format, type, idx) {
        const video = videoElements[idx];
        GLctx.texSubImage2D(target, level, 0, 0, width, height, format, type, video);
    },
    siv3dCaptureVideoFrame__sig: "viiiiiiiii",
    siv3dCaptureVideoFrame__deps: ["$videoElements"],

    siv3dQueryVideoPlaybackedTime: function(idx) {
        const video = videoElements[idx];
        return video.currentTime;
    },
    siv3dQueryVideoPlaybackedTime__sig: "di",
    siv3dQueryVideoPlaybackedTime__deps: ["$videoElements"],

    siv3dSetVideoPlaybackedTime: function(idx, time) {
        const video = videoElements[idx];
        video.currentTime = time;
    },
    siv3dSetVideoPlaybackedTime__sig: "vid",
    siv3dSetVideoPlaybackedTime__deps: ["$videoElements"],

    siv3dQueryVideoDuration: function(idx) {
        const video = videoElements[idx];
        return video.duration;
    },
    siv3dQueryVideoDuration__sig: "di",
    siv3dQueryVideoDuration__deps: ["$videoElements"],

    siv3dQueryVideoEnded: function(idx) {
        const video = videoElements[idx];
        return video.ended;
    },
    siv3dQueryVideoEnded__sig: "ii",
    siv3dQueryVideoEnded__deps: ["$videoElements"],

    siv3dQueryVideoPreference: function(idx, width, height, fps) {
        const video = videoElements[idx];

        setValue(width, video.videoWidth, 'i32');
        setValue(height, video.videoHeight, 'i32');
        setValue(fps, 29.7, 'double');
    },
    siv3dQueryVideoPlaybackedTime__sig: "viiii",
    siv3dQueryVideoPlaybackedTime__deps: ["$videoElements"],

    siv3dPlayVideo: function(idx) {
        const video = videoElements[idx];
        
        video.play().catch(function () {
                siv3dRegisterUserAction(function() {
                    video.play();
                })
            }
        );
    },
    siv3dPlayVideo__sig: "vi",
    siv3dPlayVideo__deps: ["$videoElements"],

    siv3dStopVideo: function(idx) {
        const video = videoElements[idx];
        video.pause();
    },
    siv3dStopVideo__sig: "vi",
    siv3dStopVideo__deps: ["$videoElements"],

    siv3dDestroyVideo: function(idx) {
        _siv3dStopVideo(idx);

        const video = videoElements[idx];
        if (!!video.src) {
            URL.revokeObjectURL(video.src);
        }

        delete videoElements[idx];
    },
    siv3dDestroyVideo__sig: "vi",
    siv3dDestroyVideo__deps: ["$videoElements", "siv3dStopVideo"],
});
mergeInto(LibraryManager.library, {
    siv3dWebGPUConfigureSwapchain: function(deviceId, swapChainId, descriptor) {
        var device = WebGPU["mgrDevice"].get(deviceId);
        var swapChain = WebGPU["mgrSwapChain"].get(swapChainId);
        var width = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.width, 'u32', false) }}};
        var height = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.height, 'u32', false) }}};

        var desc = {
            "device": device,
            "format": WebGPU.TextureFormat[
                {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.format, 'u32', false) }}}],
            "usage": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.usage, 'u32', false) }}},
            "size": { width, height }
        };

        swapChain["configure"](desc);
    },
    siv3dWebGPUConfigureSwapchain__sig: "viii",
    siv3dWebGPUConfigureSwapchain__deps: [ "$WebGPU" ], 
});
