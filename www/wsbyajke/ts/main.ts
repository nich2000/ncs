/// <reference path="interfaces/jquery.d.ts" />

module Maxima {
    export class Global {
        private socket: WebSocket = null;

        constructor() {
            CentrGenRep.version = '1.28.2.3245';

            $('#connect').on('click', function() {
                global.initWebSockets();
            });

            $('#disconnect').on('click', function() {
                global.disconnect();
            });

            $('#register').on('click', function() {
                global.socket.send(CentrGenRep.register());
            });

            $('#authorize').on('click', function() {
                global.socket.send(CentrGenRep.authorize());
            });

            $('#generate').on('click', function() {
                global.socket.send(CentrGenRep.generate());
            });

            this.createPacketEditor();
        }

        initWebSockets() {
            try {
                global.socket = new WebSocket($('#server').val());
                global.socket.binaryType = 'arraybuffer';

                global.socket.onmessage = function(data) {
                    var bytes: Uint8Array = new Uint8Array(data.data);
                    var array: Array<number> = Protocol.Packet.toNumbersArray(bytes);
                    var packet: Protocol.Packet = Protocol.Packet.importFromBytesArray(array);
                    var output: string = global.packetToString(packet);

                    $('#recieved').val(output);
                }

                global.socket.onopen = function() {
                    $("#recieved").val("Connection established");
                };

                global.socket.onclose = function(event) {
                    if (event.wasClean) {
                        $("#recieved").val('Connection closed');
                    } else {
                        $("#recieved").val('Connection lost\r\nCode: ' + event.code + '. Reason: ' + event.reason);
                    }
                };
            } catch (exc) {

            }
        }

        packetToString(packet: Protocol.Packet, prefix: string = ''): string {
            var result = prefix + 'Command: ' + packet.command + '; File: ' + packet.file + ';' + '\r\n';

            for (var i = 0; i < packet.linesCount; i++) {
                var line: Protocol.PacketLine = packet.getLine(i);
                result += prefix + (i + 1) + ') Line:' + '\r\n';

                for (var j = 0; j < line.wordsCount; j++) {
                    var word: Protocol.PacketWord = line.getWord(j);

                    if (word.type == Protocol.WLPType.wtInnerPkg) {
                        result += prefix + '  ' + (j + 1) + ') Word: ' + '\r\n';
                        result += global.packetToString(word.value, prefix + '    ');
                    } else {
                        result += prefix + '  ' + (j + 1) + ') Word: ' + word.value + '\r\n';
                    }
                }
            }

            return result;
        }

        disconnect(): void {
            this.socket.close();
        }

        private createPacketEditor() {
            $('#generator').append(this.packetEditor.packet());

            $('#send').on('click', function () {
                var cmd = $('#generator .command').val();
                var file = $('#generator .file').val();

                var packet: Protocol.Packet = new Protocol.Packet(cmd, file);

                var lines = $('#generator .line');
                for (var i = 0; i < lines.length; i++) {
                    var words = $(lines[i]).find('.word');
                    var line = packet.addLine();

                    for (var j = 0; j < words.length; j++) {
                        var type = $(words[j]).find('select').val();
                        var value = $(words[j]).find('input').val();

                        if (type == 0) value = null;
                        line.addWord(parseInt(type), value);
                    }
                }

                global.socket.send(packet.pack());
            });
        }

        private packetEditor = {
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

                ul.append(li)

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

                var removeWord = $('<button/>').text('remove line').on('click', function() {
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

                for (var i in Protocol.WLPType) {
                    if (!isNaN(parseInt(Protocol.WLPType[i]))) continue;
                    var option = $('<option/>').attr('value', i).text(Protocol.WLPType[i]);
                    select.append(option);
                }

                return select;
            }
        }
    }
}

var global: Maxima.Global;
$(document).ready(function () {
    global = new Maxima.Global();
});

