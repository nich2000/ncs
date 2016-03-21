var __extends = (this && this.__extends) || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};
var ws;
var dataTable;
var clientsTable;
var exec;
function init() {
    console.log("init");
    exec = new exec_t();
    ws = new web_socket_t("ws://" + location.hostname + ":5800");
    clientsTable = new clients_table_t("remote_clients", 2);
    dataTable = new data_table_t("remote_data", 2);
}
window.addEventListener("load", init, false);
var web_socket_t = (function () {
    function web_socket_t(host) {
        var _this = this;
        console.log("constructor: web_socket_t");
        this.host = host;
        this.is_connected = false;
        Signal.bind("doSend", this.doSend, this);
        var timerConnect = setInterval(function () {
            _this.createWebSocket();
        }, 3000);
    }
    web_socket_t.prototype.createWebSocket = function () {
        var _this = this;
        if ((this.socket) && (this.is_connected))
            return;
        this.socket = new WebSocket(this.host);
        this.socket.onopen = function (evt) { _this.onOpen(evt); };
        this.socket.onclose = function (evt) { _this.onClose(evt); };
        this.socket.onmessage = function (evt) { _this.onMessage(evt); };
        this.socket.onerror = function (evt) { _this.onError(evt); };
    };
    web_socket_t.prototype.doSend = function (message) {
        console.log("doSend: " + message);
        this.socket.send(message);
    };
    web_socket_t.prototype.onOpen = function (evt) {
        console.log("onOpen");
        this.is_connected = true;
        $('#connection_status').removeClass('label-danger');
        $('#connection_status').removeClass('label-warning');
        $('#connection_status').addClass('label-success');
        $("#connection_status").text('Connection: open');
    };
    web_socket_t.prototype.onClose = function (evt) {
        console.log("onClose");
        this.is_connected = false;
        $('#connection_status').removeClass('label-danger');
        $('#connection_status').removeClass('label-success');
        $('#connection_status').addClass('label-warning');
        $("#connection_status").text('Connection: close');
    };
    web_socket_t.prototype.onError = function (evt) {
        console.log(evt.data);
        this.is_connected = false;
        $('#connection_status').removeClass('label-success');
        $('#connection_status').removeClass('label-warning');
        $('#connection_status').addClass('label-danger');
        $("#connection_status").text('Connection: error');
    };
    web_socket_t.prototype.onMessage = function (evt) {
        console.log("onMessage: " + evt.data);
        Signal.emit('onMessage', evt.data);
    };
    return web_socket_t;
}());
var cell_t = (function () {
    function cell_t(id, text) {
        this.id = id;
        this.text = text;
    }
    return cell_t;
}());
var row_t = (function () {
    function row_t() {
    }
    return row_t;
}());
var table_t = (function () {
    function table_t(id, cols) {
        console.log("constructor: table_t, id: " + id);
        this.owner = $(window);
        this.table = $("#" + id);
        this.id = id;
        this.cols_count = cols;
        this.rows_count = 0;
    }
    return table_t;
}());
var clients_table_t = (function (_super) {
    __extends(clients_table_t, _super);
    function clients_table_t(id, cols) {
        _super.call(this, id, cols);
        Signal.bind("add_client", this.add_row, this);
    }
    clients_table_t.prototype.add_row = function (client) {
        var line_id = "client_" + client[0] + "_" + client[1];
        var line = $("#" + line_id);
        if (line.length == 0) {
            line = $("<tr></tr>");
            line.attr("id", line_id);
            line.click(function () {
                $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
                var cell;
                cell = $(this).find('td:last');
                var value = cell.text();
                Signal.emit("doSend", [["cmd", "activate"], ["par", value]]);
            });
            this.table.append(line);
            this.rows_count++;
        }
        var cell_id = "name_" + line_id;
        var cell = $("#" + cell_id);
        if (cell.length == 0) {
            cell = $("<td style='height:8px'></td>");
            cell.attr("id", cell_id);
            line.append(cell);
        }
        cell.text(client[0]);
        var cell_id = "id_" + line_id;
        var cell = $("#" + cell_id);
        if (cell.length == 0) {
            cell = $("<td style='height:8px'></td>");
            cell.attr("id", cell_id);
            line.append(cell);
        }
        cell.text(client[1]);
    };
    return clients_table_t;
}(table_t));
var data_table_t = (function (_super) {
    __extends(data_table_t, _super);
    function data_table_t(id, cols) {
        _super.call(this, id, cols);
        Signal.bind("add_data", this.add_row, this);
    }
    data_table_t.prototype.add_row = function (data) {
        var line_id = data[0];
        var line = $("#" + line_id);
        if (line.length == 0) {
            line = $("<tr></tr>");
            line.attr("id", line_id);
            this.table.append(line);
        }
        var cell_id = "key_" + data[0];
        var cell = $("#" + cell_id);
        if (cell.length == 0) {
            cell = $("<td style='height:8px'></td>");
            cell.attr("id", cell_id);
            line.append(cell);
        }
        cell.text(data[0]);
        cell_id = "value_" + data[0];
        cell = $("#" + cell_id);
        if (cell.length == 0) {
            cell = $("<td style='height:8px'></td>");
            cell.attr("id", cell_id);
            line.append(cell);
        }
        cell.text(data[1]);
    };
    return data_table_t;
}(table_t));
var exec_t = (function () {
    function exec_t() {
        Signal.bind("onMessage", this.doOnMessage, this);
    }
    exec_t.prototype.doOnMessage = function (message) {
        var data;
        data = JSON.parse(message);
        if (data.length == 0)
            return;
        if (data[0].length == 0)
            return;
        if (data[0][0] == "CMD") {
            $("#last_cmd").text("Command: " + data[0][1]);
            switch (data[0][1]) {
                case "clients":
                    {
                        for (var i = 1; i < data.length; i++)
                            Signal.emit("add_client", data[i][1].split('_'));
                        break;
                    }
            }
        }
        else {
            for (var i = 0; i < data.length; i++)
                Signal.emit("add_data", data[i]);
        }
    };
    return exec_t;
}());
var Signal = (function () {
    function Signal() {
    }
    Signal.bind = function (signal, method, context) {
        var tmp = {
            signal: signal,
            method: method
        };
        if (context)
            tmp.context = context;
        this.signals.push(tmp);
    };
    Signal.emit = function (signal, data) {
        for (var key in this.signals) {
            if (this.signals[key].signal == signal) {
                if (this.signals[key].context) {
                    this.signals[key].method.call(this.signals[key].context, data);
                }
                else {
                    this.signals[key].method(data);
                }
            }
        }
    };
    Signal.signals = [];
    return Signal;
}());
function asInt(value) {
    var result = [
        (value & 0x000000ff),
        (value & 0x0000ff00) >> 8,
        (value & 0x00ff0000) >> 16,
        (value & 0xff000000) >> 24
    ];
    return result;
}
function toInt(value) {
    var result = 0;
    for (var i = 0; i < 4; i++) {
        result += value[i] << (i * 8);
    }
    return result;
}
function asDouble(value) {
    var result;
    var binary = new Float64Array([value]);
    var array = new Uint8Array(binary.buffer);
    for (var i = 0; i < array.length; i++) {
        result.push(array[i]);
    }
    return result;
}
function toDouble(value) {
    var array = new Uint8Array(value);
    return new Float64Array(array.buffer)[0];
}
function toNumbersArray(value) {
    var buffer;
    for (var i = 0; i < value.length; i++) {
        buffer[i] = value[i];
    }
    return buffer;
}
function toNumberToByte(value) {
    var arr = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 'A', 'B', 'C', 'D', 'E', 'F'];
    return arr[value >> 4] + '' + arr[value & 0xF];
}
//# sourceMappingURL=script.js.map