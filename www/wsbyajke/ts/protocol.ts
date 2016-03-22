module Maxima.Protocol {
    export enum WLPType {
        wtNull,
        wtAnsiStr,
        wtWideStr,
        wtBytes,
        wtInnerPkg,
        wtInt,
        wtCardinal,
        wtBoolean,
        wtDouble,
        wtExtended,
        wtDateTime
    };

    class Utils {
        asInt(value: number): Array<number> {
            var result: Array<number> = [
                (value & 0x000000ff),
                (value & 0x0000ff00) >> 8,
                (value & 0x00ff0000) >> 16,
                (value & 0xff000000) >> 24
            ];

            return result;
        }

        toInt(value: Array<number>): number {
            var result: number = 0;

            for (var i = 0; i < 4; i++) { // 4 bytes for integer
                result += value[i] << (i * 8);
            }

            return result;
        }

        asDouble(value: number): Array<number> {
            var result: Array<number> = [];
            var binary: Float64Array = new Float64Array([value]);
            var array: Uint8Array = new Uint8Array(binary.buffer);

            for (var i = 0; i < array.length; i++) {
                result.push(array[i]);
            }

            return result;
        }

        toDouble(value: Array<number>): number {
            var array = new Uint8Array(value);
            return new Float64Array(array.buffer)[0];
        }

        asString(value: string): Array<number> {
            var result: Array<number> = [];

            for (var i = 0; i < value.length; i++) {
                var code: number = value.charCodeAt(i);

                if (code < 256) {
                    result.push(code);
                    result.push(0);
                } else {
                    var arrCode: Array<number> = this.asInt(code);

                    result.push(arrCode[0]);
                    result.push(arrCode[1]);
                }
            }

            return result;
        }

        toString(value: Array<number>): string {
            var result = '';

            for (var i = 0; i < value.length; i += 2) {
                var char: number = value[i] + (value[i + 1] << 8);
                result += String.fromCharCode(char);
            }

            return result;
        }

        toCardinal(value: Array<number>): number {
            return Math.pow(2, 8) // смещение на 8
        }
    }
    export var utils = new Utils();

    export class Packet {
        private lines: Array<PacketLine> = [];

        private _command: string;
        private _file: string;

        private _bytesArray: Array<number> = [];

        public get linesCount(): number {
            return this.lines.length;
        }

        public get command(): string {
            return this._command;
        }

        public get file(): string {
            return this._file;
        }

        constructor(command: string = '', file: string = '') {
            this._command = command;
            this._file = file;
        }

        pack(): ArrayBuffer {
            var bytesArray: Array<number> = [];

            bytesArray = bytesArray.concat(utils.asInt(this._command.length * 2));
            bytesArray = bytesArray.concat(utils.asString(this._command));

            bytesArray = bytesArray.concat(utils.asInt(this._file.length * 2));
            bytesArray = bytesArray.concat(utils.asString(this._file));

            for (var i = 0; i < this.lines.length; i++) {
                bytesArray = bytesArray.concat(this.lines[i].pack());
            }

            this._bytesArray = bytesArray;
            return new Uint8Array(bytesArray).buffer;
        }

        asBytesArray(): Array<number> {
            return this._bytesArray;
        }

        asArrayBuffer(): ArrayBuffer {
            return new Uint8Array(this._bytesArray).buffer;
        }

        clear(): Packet {
            this.lines = [];
            return this;
        }

        addLine(line: PacketLine = new PacketLine()): PacketLine {
            this.lines.push(line);
            return line;
        }

        getLine(index: number): PacketLine {
            return this.lines[index];
        }

        static toNumbersArray(value: Uint8Array): Array<number> {
            var buffer: Array<number> = [];
            for (var i = 0; i < value.length; i++) {
                buffer[i] = value[i];
            }

            return buffer;
        }

        static importFromBytesArray(value: Array<number>): Packet {
            var result: Packet;

            var length: number = utils.toInt(value.splice(0, 4));
            var command: string = utils.toString(value.splice(0, length));

            length = utils.toInt(value.splice(0, 4));
            var file: string = utils.toString(value.splice(0, length));

            result = new Packet(command, file);

            while (value.length != 0) {
                try {
                    length = utils.toInt(value.splice(0, 4));
                    result.addLine(
                        PacketLine.importFromBytesArray(
                            value.splice(0, length)
                        )
                    );
                } catch (exc) {
                    console.log('Parse error');
                    break;
                }
            }

            return result;
        }
    }

    export class PacketLine {
        private words: Array<PacketWord> = [];

        public get wordsCount(): number {
            return this.words.length;
        }

        pack(): Array<number> {
            var bytesArray: Array<number> = [];

            for (var i = 0; i < this.words.length; i++) {
                bytesArray = bytesArray.concat(this.words[i].pack());
            }

            var lineSize: Array<number> = utils.asInt(bytesArray.length);
            bytesArray = lineSize.concat(bytesArray)

            return bytesArray;
        }

        clear(): PacketLine {
            this.words = [];
            return this;
        }

        addWord(type: any, value: any = null): PacketWord {
            var word: PacketWord = new PacketWord(type, value);

            this.words.push(word);
            return word;
        }

        pushWord(word: PacketWord): PacketLine {
            this.words.push(word);
            return this;
        }

        getWord(index: number): PacketWord {
            return this.words[index];
        }

        deleteWord(index: number): PacketLine {
            this.words.splice(index, 1);
            return this;
        }

        static importFromBytesArray(value: Array<number>): PacketLine {
            var line: PacketLine = new PacketLine();

            while (value.length != 0) {
                var wordType: number = utils.toInt(value.splice(0, 1));
                var wordValue: any = null;

                switch (wordType) {
                    case WLPType.wtWideStr:
                        var length = utils.toInt(value.splice(0, 4));
                        wordValue = utils.toString(value.splice(0, length));
                        break;
                    case WLPType.wtDouble:
                        wordValue = utils.toDouble(value.splice(0, 8));
                        break;
                    case WLPType.wtInt:
                    case WLPType.wtCardinal:
                        wordValue = utils.toInt(value.splice(0, 4));
                    case WLPType.wtNull: // null integer
                        break;
                    case WLPType.wtBoolean:
                        wordValue = value.splice(0, 1)[0] == 1;
                        break;
                    case WLPType.wtInnerPkg:
                        var length = utils.toInt(value.splice(0, 4));
                        wordValue = Packet.importFromBytesArray(value.splice(0, length));
                        break;
                }

                line.pushWord(new PacketWord(wordType, wordValue));
            }

            return line;
        }
    }

    export class PacketWord {
        private _type: number;
        private _value: any;

        public get value(): any {
            return this._value;
        }

        public get type(): any {
            return this._type;
        }

        constructor(type: number, value: any = null) {
            this._type = type;
            this._value = value;
        }

        pack(): Array<number> {
            var bytesArray = [utils.asInt(this._type)[0]];

            switch (this._type) {
                case WLPType.wtWideStr:
                    bytesArray = bytesArray.concat(utils.asInt(this._value.length * 2));
                    bytesArray = bytesArray.concat(utils.asString(this._value));
                    break;
                case WLPType.wtDouble:
                    bytesArray = bytesArray.concat(utils.asDouble(this._value));
                    break;
                case WLPType.wtInt:
                case WLPType.wtCardinal: // like integer
                    bytesArray = bytesArray.concat(utils.asInt(this._value));
                case WLPType.wtNull: // null integer
                    break;
                case WLPType.wtBoolean:
                    bytesArray = bytesArray.concat(utils.asInt(this._value ? 1 : 0)[0]);
                    break;
                case WLPType.wtInnerPkg:
                    var bytes = Packet.toNumbersArray(new Uint8Array(this._value.pack()));
                    bytesArray = bytesArray.concat(utils.asInt(bytes.length));
                    bytesArray = bytesArray.concat(bytes);
                    break;
            }

            return bytesArray;
        }
    }
}