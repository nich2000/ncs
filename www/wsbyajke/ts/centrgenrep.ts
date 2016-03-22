module Maxima.CentrGenRep {
    var WLPType = Protocol.WLPType;
    export var version: string = '0.0.0.0';

    function getVersion(): Array<string> {
        var verArr: Array<string> = version.split('.');

        for (var i = verArr.length; i < 8; i++) {
            verArr.push('0');
        }

        return verArr;
    }

    export function register(): ArrayBuffer {
        var packet = new Protocol.Packet("Register");
        var line: Protocol.PacketLine;
        var version: Array<string> = getVersion();

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

        return packet.pack();
    }

    export function authorize(): ArrayBuffer {
        var packet = new Protocol.Packet("Authorize");
        var line;

        line = packet.addLine();
        line.addWord(WLPType.wtWideStr, 'Хуй.');

        return packet.pack();
    }

    export function generate(): ArrayBuffer {
        var packet = new Protocol.Packet("Generate");
        var line;

        line = packet.addLine();
        line.addWord(WLPType.wtWideStr, 'Хуюшки. у меня нет структуры.');

        return packet.pack();
    }
}