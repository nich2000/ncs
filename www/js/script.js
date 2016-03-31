var exec_t = (function () {
    function exec_t() {
        console.log("constructor: exec_t");
        Signal.bind("onMessage", this.doOnMessage, this);
    }
    exec_t.prototype.doOnMessage = function (message) {
        var data;
        data = JSON.parse(message);
        if (data.length == 0)
            return;
        if (data[0].CMD != undefined) {
            $("#last_cmd").text("Command: " + data[0].CMD);
            switch (data[0].CMD) {
                case "clients":
                    {
                        for (var i = 1; i < data.length; i++)
                            Signal.emit("add_client", data[i].PAR);
                        break;
                    }
            }
        }
        else {
            Signal.emit("add_data", data);
        }
    };
    return exec_t;
})();
var current_t;
(function (current_t) {
    current_t[current_t["none"] = 0] = "none";
    current_t[current_t["first"] = 1] = "first";
    current_t[current_t["second"] = 2] = "second";
})(current_t || (current_t = {}));
var state_t;
(function (state_t) {
    state_t[state_t["none"] = 0] = "none";
    state_t[state_t["start"] = 1] = "start";
    state_t[state_t["starting"] = 2] = "starting";
    state_t[state_t["stop"] = 3] = "stop";
    state_t[state_t["stopping"] = 4] = "stopping";
    state_t[state_t["pause"] = 5] = "pause";
    state_t[state_t["pausing"] = 6] = "pausing";
    state_t[state_t["resume"] = 7] = "resume";
    state_t[state_t["resuming"] = 8] = "resuming";
    state_t[state_t["step"] = 9] = "step";
})(state_t || (state_t = {}));
var static_filter = [
    "_ID",
    "TIM",
    "SPD",
    "HEA",
    "LAT",
    "LON"
];
var client_t = (function () {
    function client_t(id, name) {
        this._state = state_t.none;
        this._data = [];
        this._id = id;
        this._name = name;
    }
    Object.defineProperty(client_t.prototype, "id", {
        get: function () {
            return this._id;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(client_t.prototype, "state", {
        get: function () {
            return this._state;
        },
        set: function (v) {
            this._state = v;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(client_t.prototype, "name", {
        get: function () {
            return this._name;
        },
        set: function (v) {
            this._name = v;
        },
        enumerable: true,
        configurable: true
    });
    client_t.prototype.exists_data_key = function (key) {
        for (var i = void 0; i < this._data.length; i++)
            if (this._data[i].key == key)
                return true;
        return false;
    };
    client_t.prototype.get_data_by_key = function (key) {
        for (var i = 0; i < this._data.length; i++)
            if (this._data[i].key == key)
                return this._data[i];
        return undefined;
    };
    client_t.prototype.add_data = function (data) {
        var d = this.get_data_by_key(data[0]);
        if (d == undefined) {
            d = {
                key: data[0],
                value: data[1]
            };
            this._data.push(d);
        }
        else {
            d.value = data[1];
        }
    };
    return client_t;
})();
var clients_t = (function () {
    function clients_t() {
        this._clients = [];
        this._clients_table = new clients_table_t("remote_clients", 2);
        this._data_first_table = new data_table_t("remote_data_first", 2);
        this._data_second_table = new data_table_t("remote_data_second", 2);
        Signal.bind("add_client", this.add_client, this);
        Signal.bind("add_data", this.add_data, this);
    }
    Object.defineProperty(clients_t.prototype, "items", {
        get: function () {
            return this._clients;
        },
        enumerable: true,
        configurable: true
    });
    clients_t.prototype.add_client = function (data) {
        var id = data[0].PAR;
        var name = data[1].PAR;
        if (!this.exists_by_id(id)) {
            var client = new client_t(id, name);
            this._clients.push(client);
            this._clients_table.add_row(client.id, client.name);
        }
    };
    clients_t.prototype.get_by_id = function (id) {
        for (var i = 0; i < this._clients.length; i++)
            if (this._clients[i].id == id)
                return this._clients[i];
        return undefined;
    };
    clients_t.prototype.exists_by_id = function (id) {
        for (var i = 0; i < this._clients.length; i++)
            if (this._clients[i].id == id)
                return true;
        return false;
    };
    clients_t.prototype.add_data = function (data) {
        var current = current_t.none;
        for (var i_1 = 0; i_1 < data.length; i_1++) {
            if (data[i_1].ACT != undefined) {
                current = data[i_1].ACT;
                break;
            }
        }
        var id = -1;
        for (var i_2 = 0; i_2 < data.length; i_2++) {
            if (data[i_2]._ID != undefined) {
                id = data[i_2]._ID;
                break;
            }
        }
        var client = this.get_by_id(id);
        if (client == undefined)
            return;
        else
            this.switch_current(current, client);
        if (static_filter.indexOf(data[i][0]) != -1) {
            for (var i = 2; i < data.length; i++) {
                client.add_data(data);
                if (current == current_t.first)
                    this._data_first_table.add_row('first', data[i][0], data[i][1]);
                else
                    this._data_second_table.add_row('second', data[i][0], data[i][1]);
            }
        }
    };
    clients_t.prototype.switch_current = function (current, client) {
    };
    return clients_t;
})();
/// <reference path="./jquery.d.ts"/>
var element = (function () {
    function element() {
    }
    element.add = function (id, tag, owner) {
        var tmp = $(tag);
        tmp.attr("id", id);
        owner.append(tmp);
        return tmp;
    };
    element.delete = function () {
    };
    element.find_by_id = function (id) {
        var res = $("#" + id);
        if (res.length > 0)
            return res;
        else
            return undefined;
    };
    element.exists_by_id = function (id) {
        var res = $("#" + id);
        return res.length > 0;
    };
    element.get_text = function (id) {
        var res = $("#" + id);
        return res.text();
    };
    element.set_text = function (id, text) {
        var res = $("#" + id);
        res.text(text);
    };
    return element;
})();
/// <reference path="./jquery.d.ts"/>
var exec;
var clients;
var ws;
function init() {
    console.log("init");
    exec = new exec_t();
    clients = new clients_t();
    ws = new web_socket_t("ws://" + location.hostname + ":5800");
}
$(window).load(function () {
    $('body').height($(window).height());
    init();
});
$(window).resize(function () {
    $('body').height($(window).height());
});
/// <reference path="./jquery.d.ts"/>
var map_t = (function () {
    function map_t(id) {
        this._is_init = false;
        console.log("constructor: map_t, id: " + id);
        this._id = id;
        this._canvas = $("#" + id)[0];
        var canvasSupported = !!document.createElement("canvas").getContext;
        if (canvasSupported) {
            this._ctx = this._canvas.getContext('2d');
            this._height = this._canvas.height;
            this._width = this._canvas.width;
            this._is_init = true;
        }
        else {
            console.error('Can not get context');
            this._is_init = false;
        }
        this.clear();
    }
    map_t.prototype.get_height = function () {
        return this._height;
    };
    map_t.prototype.set_height = function (height) {
        this._height = height;
    };
    map_t.prototype.get_width = function () {
        return this._width;
    };
    map_t.prototype.set_width = function (width) {
        this._width = width;
    };
    map_t.prototype.get_scale = function () {
        return this._scale;
    };
    map_t.prototype.set_scale = function (height, width) {
        var tmp_scale_h = height / this._height;
        var tmp_scale_w = width / this._width;
        if (tmp_scale_h < tmp_scale_w)
            this._scale = tmp_scale_h;
        else
            this._scale = tmp_scale_w;
    };
    map_t.prototype.clear = function () {
        if (!this._is_init)
            return;
        this._ctx.clearRect(0, 0, this._canvas.width, this._canvas.height);
    };
    map_t.prototype.test_draw = function () {
        if (!this._is_init)
            return;
        this.clear();
        this._ctx.beginPath();
        this._ctx.moveTo(170, 80);
        this._ctx.bezierCurveTo(130, 100, 130, 150, 230, 150);
        this._ctx.bezierCurveTo(250, 180, 320, 180, 340, 150);
        this._ctx.bezierCurveTo(420, 150, 420, 120, 390, 100);
        this._ctx.bezierCurveTo(430, 40, 370, 30, 340, 50);
        this._ctx.bezierCurveTo(320, 5, 250, 20, 250, 50);
        this._ctx.bezierCurveTo(200, 5, 150, 20, 170, 80);
        this._ctx.closePath();
        this._ctx.lineWidth = 5;
        this._ctx.strokeStyle = 'blue';
        this._ctx.stroke();
    };
    return map_t;
})();
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
})();
/// <reference path="./jquery.d.ts"/>
var web_socket_t = (function () {
    function web_socket_t(host) {
        var _this = this;
        console.log("constructor: web_socket_t");
        this._host = host;
        this._is_connected = false;
        Signal.bind("doSend", this.doSend, this);
        var timerConnect = setInterval(function () {
            _this.createWebSocket();
        }, 3000);
    }
    web_socket_t.prototype.createWebSocket = function () {
        var _this = this;
        if ((this._socket) && (this._is_connected))
            return;
        this._socket = new WebSocket(this._host);
        this._socket.onopen = function (evt) { _this.onOpen(evt); };
        this._socket.onclose = function (evt) { _this.onClose(evt); };
        this._socket.onmessage = function (evt) { _this.onMessage(evt); };
        this._socket.onerror = function (evt) { _this.onError(evt); };
    };
    web_socket_t.prototype.doSend = function (message) {
        console.log("doSend: " + message);
        message = JSON.stringify(message);
        this._socket.send(message);
    };
    web_socket_t.prototype.onOpen = function (evt) {
        console.log("onOpen");
        this._is_connected = true;
        $('#connection_status').removeClass('label-danger');
        $('#connection_status').removeClass('label-warning');
        $('#connection_status').addClass('label-success');
        element.set_text("connection_status", 'Connection: open');
    };
    web_socket_t.prototype.onClose = function (evt) {
        console.log("onClose");
        this._is_connected = false;
        $('#connection_status').removeClass('label-danger');
        $('#connection_status').removeClass('label-success');
        $('#connection_status').addClass('label-warning');
        element.set_text("connection_status", 'Connection: close');
    };
    web_socket_t.prototype.onError = function (evt) {
        console.log(evt.data);
        this._is_connected = false;
        $('#connection_status').removeClass('label-success');
        $('#connection_status').removeClass('label-warning');
        $('#connection_status').addClass('label-danger');
        element.set_text("connection_status", 'Connection: error');
    };
    web_socket_t.prototype.onMessage = function (evt) {
        console.log("onMessage: " + evt.data);
        Signal.emit('onMessage', evt.data);
    };
    return web_socket_t;
})();
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
var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};
var custom_t = (function () {
    function custom_t(id, tag, owner) {
        this._id = id;
        this._owner = owner;
        this._self = element.find_by_id(id);
        if (this._self == undefined)
            this._self = element.add(id, tag, owner);
    }
    Object.defineProperty(custom_t.prototype, "id", {
        get: function () {
            return this._id;
        },
        enumerable: true,
        configurable: true
    });
    return custom_t;
})();
var cell_t = (function (_super) {
    __extends(cell_t, _super);
    function cell_t(id, owner) {
        _super.call(this, id, "<td/>", owner);
    }
    Object.defineProperty(cell_t.prototype, "text", {
        get: function () {
            return this._self.text();
        },
        set: function (v) {
            this._self.text(v);
        },
        enumerable: true,
        configurable: true
    });
    return cell_t;
})(custom_t);
var row_t = (function (_super) {
    __extends(row_t, _super);
    function row_t(id, owner) {
        _super.call(this, id, "<tr/>", owner);
        this._cells = [];
        var value = '1234';
        this._self.click(function () {
            $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
            Signal.emit("doSend", [["cmd", "activate"], ["par", value], ["par", "first"]]);
        });
    }
    Object.defineProperty(row_t.prototype, "cells", {
        get: function () {
            return this._cells;
        },
        enumerable: true,
        configurable: true
    });
    row_t.prototype.add_cell = function (id, text) {
        if (!element.exists_by_id(id)) {
            var cell = new cell_t(id, this._self);
            this._cells.push(cell);
            cell.text = text;
        }
    };
    return row_t;
})(custom_t);
var table_t = (function (_super) {
    __extends(table_t, _super);
    function table_t(id, owner, cols_count) {
        _super.call(this, id, "<table/>", owner);
        this._rows = [];
        this._cols_count = cols_count;
    }
    Object.defineProperty(table_t.prototype, "rows", {
        get: function () {
            return this._rows;
        },
        enumerable: true,
        configurable: true
    });
    table_t.prototype.do_add_row = function (id) {
        if (!element.exists_by_id(id)) {
            var row = new row_t(id, this._self);
            this._rows.push(row);
            return row;
        }
    };
    return table_t;
})(custom_t);
var clients_table_t = (function (_super) {
    __extends(clients_table_t, _super);
    function clients_table_t(id, cols_count) {
        _super.call(this, id, $(window), cols_count);
    }
    clients_table_t.prototype.add_row = function (id, name) {
        var row_id = 'client_' + String(id) + '_' + name;
        var row = _super.prototype.do_add_row.call(this, row_id);
        var cell_id = 'client_name_' + String(id);
        row.add_cell(cell_id, name);
        cell_id = 'client_id_' + String(id);
        row.add_cell(cell_id, String(id));
    };
    return clients_table_t;
})(table_t);
var data_table_t = (function (_super) {
    __extends(data_table_t, _super);
    function data_table_t(id, cols_count) {
        _super.call(this, id, $(window), cols_count);
    }
    data_table_t.prototype.add_row = function (prefix, key, value) {
        var row_id = 'data_' + prefix + '_' + key + '_' + value;
        var row = _super.prototype.do_add_row.call(this, row_id);
        var cell_id = 'data_' + prefix + '_key_' + key;
        row.add_cell(cell_id, key);
        cell_id = 'data_' + prefix + '_value_' + key;
        row.add_cell(cell_id, value);
    };
    return data_table_t;
})(table_t);
//# sourceMappingURL=script.js.map