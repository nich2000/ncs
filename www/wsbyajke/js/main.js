var Maxima;
(function (Maxima) {
    var Protocol;
    (function (Protocol) {
        (function (WLPType) {
            WLPType[WLPType["wtNull"] = 0] = "wtNull";
            WLPType[WLPType["wtAnsiStr"] = 1] = "wtAnsiStr";
            WLPType[WLPType["wtWideStr"] = 2] = "wtWideStr";
            WLPType[WLPType["wtBytes"] = 3] = "wtBytes";
            WLPType[WLPType["wtInnerPkg"] = 4] = "wtInnerPkg";
            WLPType[WLPType["wtInt"] = 5] = "wtInt";
            WLPType[WLPType["wtCardinal"] = 6] = "wtCardinal";
            WLPType[WLPType["wtBoolean"] = 7] = "wtBoolean";
            WLPType[WLPType["wtDouble"] = 8] = "wtDouble";
            WLPType[WLPType["wtExtended"] = 9] = "wtExtended";
            WLPType[WLPType["wtDateTime"] = 10] = "wtDateTime";
        })(Protocol.WLPType || (Protocol.WLPType = {}));
        var WLPType = Protocol.WLPType;
        ;
        var Utils = (function () {
            function Utils() {
            }
            Utils.prototype.asInt = function (value) {
                var result = [
                    (value & 0x000000ff),
                    (value & 0x0000ff00) >> 8,
                    (value & 0x00ff0000) >> 16,
                    (value & 0xff000000) >> 24
                ];
                return result;
            };
            Utils.prototype.toInt = function (value) {
                var result = 0;
                for (var i = 0; i < 4; i++) {
                    result += value[i] << (i * 8);
                }
                return result;
            };
            Utils.prototype.asDouble = function (value) {
                var result = [];
                var binary = new Float64Array([value]);
                var array = new Uint8Array(binary.buffer);
                for (var i = 0; i < array.length; i++) {
                    result.push(array[i]);
                }
                return result;
            };
            Utils.prototype.toDouble = function (value) {
                var array = new Uint8Array(value);
                return new Float64Array(array.buffer)[0];
            };
            Utils.prototype.asString = function (value) {
                var result = [];
                for (var i = 0; i < value.length; i++) {
                    var code = value.charCodeAt(i);
                    if (code < 256) {
                        result.push(code);
                        result.push(0);
                    }
                    else {
                        var arrCode = this.asInt(code);
                        result.push(arrCode[0]);
                        result.push(arrCode[1]);
                    }
                }
                return result;
            };
            Utils.prototype.toString = function (value) {
                var result = '';
                for (var i = 0; i < value.length; i += 2) {
                    var char = value[i] + (value[i + 1] << 8);
                    result += String.fromCharCode(char);
                }
                return result;
            };
            Utils.prototype.toCardinal = function (value) {
                return Math.pow(2, 8);
            };
            return Utils;
        })();
        Protocol.utils = new Utils();
        var Packet = (function () {
            function Packet(command, file) {
                if (command === void 0) { command = ''; }
                if (file === void 0) { file = ''; }
                this.lines = [];
                this._bytesArray = [];
                this._command = command;
                this._file = file;
            }
            Object.defineProperty(Packet.prototype, "linesCount", {
                get: function () {
                    return this.lines.length;
                },
                enumerable: true,
                configurable: true
            });
            Object.defineProperty(Packet.prototype, "command", {
                get: function () {
                    return this._command;
                },
                enumerable: true,
                configurable: true
            });
            Object.defineProperty(Packet.prototype, "file", {
                get: function () {
                    return this._file;
                },
                enumerable: true,
                configurable: true
            });
            Packet.prototype.pack = function () {
                var bytesArray = [];
                bytesArray = bytesArray.concat(Protocol.utils.asInt(this._command.length * 2));
                bytesArray = bytesArray.concat(Protocol.utils.asString(this._command));
                bytesArray = bytesArray.concat(Protocol.utils.asInt(this._file.length * 2));
                bytesArray = bytesArray.concat(Protocol.utils.asString(this._file));
                for (var i = 0; i < this.lines.length; i++) {
                    bytesArray = bytesArray.concat(this.lines[i].pack());
                }
                this._bytesArray = bytesArray;
                return new Uint8Array(bytesArray).buffer;
            };
            Packet.prototype.asBytesArray = function () {
                return this._bytesArray;
            };
            Packet.prototype.asArrayBuffer = function () {
                return new Uint8Array(this._bytesArray).buffer;
            };
            Packet.prototype.clear = function () {
                this.lines = [];
                return this;
            };
            Packet.prototype.addLine = function (line) {
                if (line === void 0) { line = new PacketLine(); }
                this.lines.push(line);
                return line;
            };
            Packet.prototype.getLine = function (index) {
                return this.lines[index];
            };
            Packet.toNumbersArray = function (value) {
                var buffer = [];
                for (var i = 0; i < value.length; i++) {
                    buffer[i] = value[i];
                }
                return buffer;
            };
            Packet.importFromBytesArray = function (value) {
                var result;
                var length = Protocol.utils.toInt(value.splice(0, 4));
                var command = Protocol.utils.toString(value.splice(0, length));
                length = Protocol.utils.toInt(value.splice(0, 4));
                var file = Protocol.utils.toString(value.splice(0, length));
                result = new Packet(command, file);
                while (value.length != 0) {
                    try {
                        length = Protocol.utils.toInt(value.splice(0, 4));
                        result.addLine(PacketLine.importFromBytesArray(value.splice(0, length)));
                    }
                    catch (exc) {
                        console.log('Parse error');
                        break;
                    }
                }
                return result;
            };
            return Packet;
        })();
        Protocol.Packet = Packet;
        var PacketLine = (function () {
            function PacketLine() {
                this.words = [];
            }
            Object.defineProperty(PacketLine.prototype, "wordsCount", {
                get: function () {
                    return this.words.length;
                },
                enumerable: true,
                configurable: true
            });
            PacketLine.prototype.pack = function () {
                var bytesArray = [];
                for (var i = 0; i < this.words.length; i++) {
                    bytesArray = bytesArray.concat(this.words[i].pack());
                }
                var lineSize = Protocol.utils.asInt(bytesArray.length);
                bytesArray = lineSize.concat(bytesArray);
                return bytesArray;
            };
            PacketLine.prototype.clear = function () {
                this.words = [];
                return this;
            };
            PacketLine.prototype.addWord = function (type, value) {
                if (value === void 0) { value = null; }
                var word = new PacketWord(type, value);
                this.words.push(word);
                return word;
            };
            PacketLine.prototype.pushWord = function (word) {
                this.words.push(word);
                return this;
            };
            PacketLine.prototype.getWord = function (index) {
                return this.words[index];
            };
            PacketLine.prototype.deleteWord = function (index) {
                this.words.splice(index, 1);
                return this;
            };
            PacketLine.importFromBytesArray = function (value) {
                var line = new PacketLine();
                while (value.length != 0) {
                    var wordType = Protocol.utils.toInt(value.splice(0, 1));
                    var wordValue = null;
                    switch (wordType) {
                        case WLPType.wtWideStr:
                            var length = Protocol.utils.toInt(value.splice(0, 4));
                            wordValue = Protocol.utils.toString(value.splice(0, length));
                            break;
                        case WLPType.wtDouble:
                            wordValue = Protocol.utils.toDouble(value.splice(0, 8));
                            break;
                        case WLPType.wtInt:
                        case WLPType.wtCardinal:
                            wordValue = Protocol.utils.toInt(value.splice(0, 4));
                        case WLPType.wtNull:
                            break;
                        case WLPType.wtBoolean:
                            wordValue = value.splice(0, 1)[0] == 1;
                            break;
                        case WLPType.wtInnerPkg:
                            var length = Protocol.utils.toInt(value.splice(0, 4));
                            wordValue = Packet.importFromBytesArray(value.splice(0, length));
                            break;
                    }
                    line.pushWord(new PacketWord(wordType, wordValue));
                }
                return line;
            };
            return PacketLine;
        })();
        Protocol.PacketLine = PacketLine;
        var PacketWord = (function () {
            function PacketWord(type, value) {
                if (value === void 0) { value = null; }
                this._type = type;
                this._value = value;
            }
            Object.defineProperty(PacketWord.prototype, "value", {
                get: function () {
                    return this._value;
                },
                enumerable: true,
                configurable: true
            });
            Object.defineProperty(PacketWord.prototype, "type", {
                get: function () {
                    return this._type;
                },
                enumerable: true,
                configurable: true
            });
            PacketWord.prototype.pack = function () {
                var bytesArray = [Protocol.utils.asInt(this._type)[0]];
                switch (this._type) {
                    case WLPType.wtWideStr:
                        bytesArray = bytesArray.concat(Protocol.utils.asInt(this._value.length * 2));
                        bytesArray = bytesArray.concat(Protocol.utils.asString(this._value));
                        break;
                    case WLPType.wtDouble:
                        bytesArray = bytesArray.concat(Protocol.utils.asDouble(this._value));
                        break;
                    case WLPType.wtInt:
                    case WLPType.wtCardinal:
                        bytesArray = bytesArray.concat(Protocol.utils.asInt(this._value));
                    case WLPType.wtNull:
                        break;
                    case WLPType.wtBoolean:
                        bytesArray = bytesArray.concat(Protocol.utils.asInt(this._value ? 1 : 0)[0]);
                        break;
                    case WLPType.wtInnerPkg:
                        var bytes = Packet.toNumbersArray(new Uint8Array(this._value.pack()));
                        bytesArray = bytesArray.concat(Protocol.utils.asInt(bytes.length));
                        bytesArray = bytesArray.concat(bytes);
                        break;
                }
                return bytesArray;
            };
            return PacketWord;
        })();
        Protocol.PacketWord = PacketWord;
    })(Protocol = Maxima.Protocol || (Maxima.Protocol = {}));
})(Maxima || (Maxima = {}));
var Maxima;
(function (Maxima) {
    var CentrGenRep;
    (function (CentrGenRep) {
        var WLPType = Maxima.Protocol.WLPType;
        CentrGenRep.version = '0.0.0.0';
        function getVersion() {
            var verArr = CentrGenRep.version.split('.');
            for (var i = verArr.length; i < 8; i++) {
                verArr.push('0');
            }
            return verArr;
        }
        function register() {
            var packet = new Maxima.Protocol.Packet("Register");
            var line;
            var version = getVersion();
			line = packet.addLine();			
			line.addWord(WLPType.wtBoolean, false);			
            line = packet.addLine();
            line.addWord(WLPType.wtWideStr, 'TCentrGenRepProgram');
            line.addWord(WLPType.wtNull);
            line.addWord(WLPType.wtBoolean, true);
            line = packet.addLine();
            for (var i = 0; i < 8; i++) {
                line.addWord(WLPType.wtInt, parseInt(version[i]));
            }
            line = packet.addLine();
            line.addWord(WLPType.wtCardinal, 33554430);
            line.addWord(WLPType.wtCardinal, 33554430);
            line.addWord(WLPType.wtCardinal, 16777215);
			
            line = packet.addLine();
            line.addWord(WLPType.wtWideStr, 'GenReportsCentr');
            line.addWord(WLPType.wtCardinal, 16777215);
			
			line = packet.addLine();
			line.addWord(WLPType.wtBoolean, true);
			
            return packet.pack();
        }
        CentrGenRep.register = register;
        function authorize() {
            var packet = new Maxima.Protocol.Packet("Authorization");
            var line;
            line = packet.addLine();
            line.addWord(WLPType.wtBoolean, true);
			
            line = packet.addLine();
            line.addWord(WLPType.wtWideStr, 'admin');
            line.addWord(WLPType.wtWideStr, 'admin');
            return packet.pack();
        }
        CentrGenRep.authorize = authorize;
        function generate() {
            var packet = new Maxima.Protocol.Packet("Generate");
            var line;
            line = packet.addLine();
            line.addWord(WLPType.wtWideStr, 'Хуюшки. у меня нет структуры.');
            return packet.pack();
        }
        CentrGenRep.generate = generate;
    })(CentrGenRep = Maxima.CentrGenRep || (Maxima.CentrGenRep = {}));
})(Maxima || (Maxima = {}));
/// <reference path="interfaces/jquery.d.ts" />
var Maxima;
(function (Maxima) {
    var Global = (function () {
        function Global() {
            this.socket = null;
            this.packetEditor = {
                packet: function () {
                    var ul = $('<ul/>');
                    var li = $('<li/>');
                    var command = $('<input/>').addClass('command');
                    var file = $('<input/>').addClass('file');
                    var addLine = $('<button/>').text("add line").on('click', function () {
                        var li = $('<li/>');
                        li.append(global.packetEditor.line());
                        ul.append(li);
                    });
                    li
                        .append('Command: ')
                        .append(command)
                        .append('File: ')
                        .append(file)
                        .append(addLine);
                    ul.append(li);
                    return ul;
                },
                line: function () {
                    var span = $('<span/>');
                    var ul = $('<ul/>').addClass('line');
                    var addWord = $('<button/>').text('add word').on('click', function () {
                        var li = $('<li/>');
                        li.append(global.packetEditor.word());
                        ul.append(li);
                    });
                    var removeLine = $('<button/>').text('remove line').on('click', function () {
                        span.parent().remove();
                    });
                    span
                        .append('Line: ')
                        .append(addWord)
                        .append(removeLine)
                        .append('<br>')
                        .append(ul)
                        .append('<br/>');
                    return span;
                },
                word: function () {
                    var span = $('<span/>').addClass('word');
                    var type = global.packetEditor.types();
                    var value = $('<input/>');
                    var removeWord = $('<button/>').text('remove line').on('click', function () {
                        span.parent().remove();
                    });
                    span
                        .append(type)
                        .append(value)
                        .append(removeWord)
                        .append('<br>');
                    return span;
                },
                types: function () {
                    var select = $('<select/>');
                    for (var i in Maxima.Protocol.WLPType) {
                        if (!isNaN(parseInt(Maxima.Protocol.WLPType[i])))
                            continue;
                        var option = $('<option/>').attr('value', i).text(Maxima.Protocol.WLPType[i]);
                        select.append(option);
                    }
                    return select;
                }
            };
            Maxima.CentrGenRep.version = '1.28.2.3245';
            $('#connect').on('click', function () {
                global.initWebSockets();
            });
            $('#disconnect').on('click', function () {
                global.disconnect();
            });
            $('#register').on('click', function () {
                global.socket.send(Maxima.CentrGenRep.register());
            });
            $('#authorize').on('click', function () {
                global.socket.send(Maxima.CentrGenRep.authorize());
            });
            $('#generate').on('click', function () {
                global.socket.send(Maxima.CentrGenRep.generate());
            });
            this.createPacketEditor();
        }
        Global.prototype.initWebSockets = function () {
            try {
                global.socket = new WebSocket($('#server').val());
                global.socket.binaryType = 'arraybuffer';
                global.socket.onmessage = function (data) {
                    var bytes = new Uint8Array(data.data);
                    var array = Maxima.Protocol.Packet.toNumbersArray(bytes);
                    var packet = Maxima.Protocol.Packet.importFromBytesArray(array);
                    var output = global.packetToString(packet);
                    $('#recieved').val(output);
                };
                global.socket.onopen = function () {
                    $("#recieved").val("Connection established");
                };
                global.socket.onclose = function (event) {
                    if (event.wasClean) {
                        $("#recieved").val('Connection closed');
                    }
                    else {
                        $("#recieved").val('Connection lost\r\nCode: ' + event.code + '. Reason: ' + event.reason);
                    }
                };
            }
            catch (exc) {
            }
        };
        Global.prototype.packetToString = function (packet, prefix) {
            if (prefix === void 0) { prefix = ''; }
            var result = prefix + 'Command: ' + packet.command + '; File: ' + packet.file + ';' + '\r\n';
            for (var i = 0; i < packet.linesCount; i++) {
                var line = packet.getLine(i);
                result += prefix + (i + 1) + ') Line:' + '\r\n';
                for (var j = 0; j < line.wordsCount; j++) {
                    var word = line.getWord(j);
                    if (word.type == Maxima.Protocol.WLPType.wtInnerPkg) {
                        result += prefix + '  ' + (j + 1) + ') Word: ' + '\r\n';
                        result += global.packetToString(word.value, prefix + '    ');
                    }
                    else {
                        result += prefix + '  ' + (j + 1) + ') Word: ' + word.value + '\r\n';
                    }
                }
            }
            return result;
        };
        Global.prototype.disconnect = function () {
            this.socket.close();
        };
        Global.prototype.createPacketEditor = function () {
            $('#generator').append(this.packetEditor.packet());
            $('#send').on('click', function () {
                var cmd = $('#generator .command').val();
                var file = $('#generator .file').val();
                var packet = new Maxima.Protocol.Packet(cmd, file);
                var lines = $('#generator .line');
                for (var i = 0; i < lines.length; i++) {
                    var words = $(lines[i]).find('.word');
                    var line = packet.addLine();
                    for (var j = 0; j < words.length; j++) {
                        var type = $(words[j]).find('select').val();
                        var value = $(words[j]).find('input').val();
                        if (type == 0)
                            value = null;
                        line.addWord(parseInt(type), value);
                    }
                }
                global.socket.send(packet.pack());
            });
        };
        return Global;
    })();
    Maxima.Global = Global;
})(Maxima || (Maxima = {}));
var global;
$(document).ready(function () {
    global = new Maxima.Global();
});
//# sourceMappingURL=main.js.map