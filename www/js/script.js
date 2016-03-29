var exec_t = (function () {
    function exec_t() {
        this.static_filter = [
            "_ID",
            "TIM",
            "SPD",
            "HEA",
            "LAT",
            "LON"
        ];
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
                            Signal.emit("add_client", data[i]);
                        break;
                    }
            }
        }
        else {
        }
    };
    return exec_t;
})();
var state_t;
(function (state_t) {
    state_t[state_t["state_none"] = 0] = "state_none";
    state_t[state_t["state_start"] = 1] = "state_start";
    state_t[state_t["state_starting"] = 2] = "state_starting";
    state_t[state_t["state_stop"] = 3] = "state_stop";
    state_t[state_t["state_stopping"] = 4] = "state_stopping";
    state_t[state_t["state_pause"] = 5] = "state_pause";
    state_t[state_t["state_pausing"] = 6] = "state_pausing";
    state_t[state_t["state_resume"] = 7] = "state_resume";
    state_t[state_t["state_resuming"] = 8] = "state_resuming";
})(state_t || (state_t = {}));
var client_t = (function () {
    function client_t(id, name) {
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
    return client_t;
})();
var clients_t = (function () {
    function clients_t() {
    }
    return clients_t;
})();
/// <reference path="./jquery.d.ts"/>
var ws;
var data_table_first;
var data_table_second;
var clientsTable;
var map;
var exec;
function init() {
    console.log("init");
    exec = new exec_t();
    ws = new web_socket_t("ws://" + location.hostname + ":5800");
    clientsTable = new clients_table_t("remote_clients", $(window), 2);
    data_table_first = new data_table_t("remote_data_first", $(window), 2);
    data_table_second = new data_table_t("remote_data_second", $(window), 2);
    map = new map_t('canvas');
    map.test_draw();
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
/*
class Socket
{
  public onmessage(data: any): void
  {
    Signal.emit('onmessage', data);
  }
}
class OtherClass
{
  public constructor()
  {
    Signal.bind('onmessage', proceedData);
  }
  public proceedData(data: any)
  {
    console.log(data);
  }
}
*/
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
        message = JSON.stringify(message);
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
/// <reference path="./jquery.d.ts"/>
var custom_t = (function () {
    function custom_t(id, owner) {
        this._id = id;
        this._owner = owner;
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
        _super.call(this, id, owner);
        this._cell = $("<tr></tr>");
        this._cell.attr("id", this.id);
    }
    Object.defineProperty(cell_t.prototype, "text", {
        get: function () {
            return this._cell.text();
        },
        set: function (v) {
            this._cell.text(v);
        },
        enumerable: true,
        configurable: true
    });
    return cell_t;
})(custom_t);
var row_t = (function (_super) {
    __extends(row_t, _super);
    function row_t(id, owner) {
        _super.call(this, id, owner);
        this._row = $("<tr></tr>");
        this._row.attr("id", this.id);
        this._row.click(function () {
            $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
            var cell = $(this).find('td:last');
            var value = cell.text();
            Signal.emit("doSend", [["cmd", "activate"], ["par", value], ["par", "on"]]);
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
    };
    row_t.prototype.exists_cell = function (id) {
        return $("#" + id).length > 0;
    };
    return row_t;
})(custom_t);
var table_t = (function (_super) {
    __extends(table_t, _super);
    function table_t(id, owner, cols_count) {
        console.log("constructor: table_t, id: " + id);
        _super.call(this, id, owner);
        this._table = $("#" + this.id);
        this._cols_count = cols_count;
    }
    Object.defineProperty(table_t.prototype, "rows", {
        get: function () {
            return this._rows;
        },
        enumerable: true,
        configurable: true
    });
    table_t.prototype.add_row = function (data) {
    };
    table_t.prototype.exists_row = function (id) {
        return $("#" + this.id).length > 0;
    };
    return table_t;
})(custom_t);
var clients_table_t = (function (_super) {
    __extends(clients_table_t, _super);
    function clients_table_t(id, owner, cols_count) {
        _super.call(this, id, owner, cols_count);
        Signal.bind("add_client", this.add_client, this);
    }
    clients_table_t.prototype.add_client = function (data) {
        if (data.PAR == undefined)
            return;
        this.add_row(data.PAR);
    };
    clients_table_t.prototype.add_row = function (data) {
        var client = new client_t(data[0].PAR, data[1].PAR);
        var row_id = "client_" + client.id + "_" + client.name;
        if (!this.exists_row(row_id)) {
        }
    };
    return clients_table_t;
})(table_t);
var data_table_t = (function (_super) {
    __extends(data_table_t, _super);
    function data_table_t(id, owner, cols_count) {
        _super.call(this, id, owner, cols_count);
        Signal.bind("add_data", this.add_row, this);
    }
    data_table_t.prototype.add_row = function (data) {
        // var line_id = data[0];
        // var line = $("#" + line_id);
        // if (line.length == 0) {
        //   line = $("<tr></tr>");
        //   line.attr("id", line_id);
        //   this.table.append(line);
        // }
    };
    return data_table_t;
})(table_t);
//# sourceMappingURL=script.js.map