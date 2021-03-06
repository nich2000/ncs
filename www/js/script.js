var exec_t = (function () {
    function exec_t() {
        console.log("constructor: exec_t");
    }
    return exec_t;
})();
var connect_t;
(function (connect_t) {
    connect_t[connect_t["unknown"] = 0] = "unknown";
    connect_t[connect_t["connected"] = 1] = "connected";
    connect_t[connect_t["disconnected"] = 2] = "disconnected";
})(connect_t || (connect_t = {}));
var active_t;
(function (active_t) {
    active_t[active_t["next"] = -1] = "next";
    active_t[active_t["unknown"] = 0] = "unknown";
    active_t[active_t["first"] = 1] = "first";
    active_t[active_t["second"] = 2] = "second";
})(active_t || (active_t = {}));
;
var state_t;
(function (state_t) {
    state_t[state_t["unknown"] = 0] = "unknown";
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
var register_t;
(function (register_t) {
    register_t[register_t["none"] = 0] = "none";
    register_t[register_t["ok"] = 1] = "ok";
})(register_t || (register_t = {}));
;
var static_filter = [
    "_ID",
    "TIM",
    "T_S",
    "CNT",
    "SPD",
    "HEA",
    "LAT",
    "LON"
];
var client_t = (function () {
    function client_t(id, name) {
        this._id = -1;
        this._name = "noname";
        this._session = "nosession";
        this._state = state_t.unknown;
        this._connect = connect_t.unknown;
        this._active = active_t.unknown;
        this._register = register_t.none;
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
    Object.defineProperty(client_t.prototype, "session", {
        get: function () {
            return this._session;
        },
        set: function (v) {
            this._session = v;
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
    Object.defineProperty(client_t.prototype, "connect", {
        get: function () {
            return this._connect;
        },
        set: function (v) {
            this._connect = v;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(client_t.prototype, "active", {
        get: function () {
            return this._active;
        },
        set: function (v) {
            this._active = v;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(client_t.prototype, "register", {
        get: function () {
            return this._register;
        },
        set: function (v) {
            this._register = v;
        },
        enumerable: true,
        configurable: true
    });
    client_t.prototype.exists_data_key = function (key) {
        return false;
    };
    client_t.prototype.get_data_by_key = function (key) {
        return undefined;
    };
    client_t.prototype.add_data = function (data) {
    };
    return client_t;
})();
var clients_t = (function () {
    function clients_t() {
        this._clients = [];
        this._clients_table = undefined;
        this._data_first_table = undefined;
        this._data_second_table = undefined;
        console.log("constructor, clients_t");
        try {
            this._clients_table = new clients_table_t("remote_clients", 3);
        }
        catch (e) {
        }
        try {
            this._data_first_table = new data_table_t("remote_data_first", 2);
        }
        catch (e) {
        }
        try {
            this._data_second_table = new data_table_t("remote_data_second", 2);
        }
        catch (e) {
        }
        Signal.bind("clients", this.refresh_clients, this);
        Signal.bind("add_data", this.add_data, this);
    }
    Object.defineProperty(clients_t.prototype, "items", {
        get: function () {
            return this._clients;
        },
        enumerable: true,
        configurable: true
    });
    clients_t.prototype.refresh_clients = function (data) {
        for (var i = 0; i < data.length; i++) {
            this.add_client(data[i].PAR);
        }
    };
    clients_t.prototype.add_client = function (data) {
        var id = data[0]._ID;
        var name = data[1].NAM;
        var session = data[2].SES;
        var state = parseInt(data[3].STA);
        var connect = parseInt(data[4].CON);
        var active = parseInt(data[5].ACT);
        var register = parseInt(data[6].REG);
        var client = this.get_client_by_id(id);
        if (client == undefined) {
            client = new client_t(id, name);
            this._clients.push(client);
            var row = this._clients_table.add_client(client);
        }
        client.session = session;
        client.connect = connect;
        client.state = state;
        client.active = active;
        client.register = register;
        this.switch_current(active, client);
        this._clients_table.state_client(client, state);
        this._clients_table.active_client(client, active);
        this._clients_table.register_client(client, register);
        return client;
    };
    clients_t.prototype.get_client_by_id = function (id) {
        for (var i = 0; i < this._clients.length; i++)
            if (this._clients[i].id == id)
                return this._clients[i];
        return undefined;
    };
    clients_t.prototype.get_client_by_name = function (name) {
        for (var i = 0; i < this._clients.length; i++)
            if (this._clients[i].name == name)
                return this._clients[i];
        return undefined;
    };
    clients_t.prototype.exists_client_by_id = function (id) {
        for (var i = 0; i < this._clients.length; i++)
            if (this._clients[i].id == id)
                return true;
        return false;
    };
    clients_t.prototype.add_data = function (data) {
        var active = active_t.unknown;
        for (var i_1 = data.length - 1; i_1 >= 0; i_1--) {
            if (data[i_1].ACT != undefined) {
                active = data[i_1].ACT;
                break;
            }
        }
        var data_table = undefined;
        var prefix;
        if (active == active_t.first) {
            data_table = this._data_first_table;
            prefix = "first";
        }
        else if (active == active_t.second) {
            data_table = this._data_second_table;
            prefix = "second";
        }
        else
            return;
        try {
            var begin = new Date();
            Signal.emit("add_position", data);
            var end = new Date;
            profiler.text(end - begin);
        }
        catch (e) {
        }
        if (data_table == undefined)
            return;
        var id = "";
        for (var i_2 = 0; i_2 < data.length; i_2++) {
            if (data[i_2]._ID != undefined) {
                id = data[i_2]._ID;
                break;
            }
        }
        for (var i = 0; i < data.length; i++) {
            if (static_filter.indexOf(Object.keys(data[i])[0]) != -1) {
                var param = Object.keys(data[i])[0];
                var value = data[i][param];
                data_table.add_row(prefix, param, value);
            }
        }
    };
    clients_t.prototype.switch_current = function (current, client) {
        var photo = "/pilots/" + client.name + ".jpg";
        var info = "/pilots/" + client.name + "_info.html";
        if (current == active_t.first) {
            element.set_src("first_pilot_photo", photo);
            element.set_src("first_pilot_info", info);
        }
        else if (current == active_t.second) {
            element.set_src("second_pilot_photo", photo);
            element.set_src("second_pilot_info", info);
        }
    };
    clients_t.prototype.select_client = function (id) {
        console.log("select_client: " + id);
        var client = this.get_client_by_id(parseInt(id));
        $.get("command?cmd=ws_activate&par=" + client.name + "&par=" + client.id + "&par=next", function (data) {
        });
    };
    return clients_t;
})();
/// <reference path="./jquery.d.ts"/>
var pos_t;
(function (pos_t) {
    pos_t[pos_t["append"] = 0] = "append";
    pos_t[pos_t["prepend"] = 1] = "prepend";
})(pos_t || (pos_t = {}));
var element = (function () {
    function element() {
    }
    element.add = function (id, tag, owner, pos) {
        if (pos === void 0) { pos = pos_t.append; }
        var tmp = this.find_by_id(id);
        if (tmp == undefined) {
            tmp = $(tag);
            tmp.attr("id", id);
            if (pos == pos_t.append)
                owner.append(tmp);
            else
                owner.prepend(tmp);
        }
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
    element.set_src = function (id, src) {
        var res = $("#" + id);
        res.attr("src", src);
    };
    return element;
})();
/// <reference path="./jquery.d.ts"/>
var use_clients = 1;
var use_map = 1;
var use_ws = 1;
var clients = undefined;
var ws = undefined;
var map = undefined;
var profiler = undefined;
function init() {
    profiler = $("#profiler");
    console.log("init");
    if (use_clients)
        clients = new clients_t();
    else
        console.log("clients not enabled");
    if (use_map) {
        map = new map_t("canvas_map");
    }
    else
        console.log("map not enabled");
    if (use_ws)
        ws = new web_socket_t("ws://" + location.hostname + ":5800");
    else
        console.log("ws not enabled");
    console.log("init success");
}
function test() {
}
$(window).load(function () {
    $("body").height($(window).height());
    init();
});
$(window).resize(function () {
    $("body").height($(window).height());
});
/// <reference path="./jquery.d.ts"/>
var map_kind_t;
(function (map_kind_t) {
    map_kind_t[map_kind_t["border"] = 0] = "border";
    map_kind_t[map_kind_t["sector"] = 1] = "sector";
    map_kind_t[map_kind_t["start"] = 2] = "start";
    map_kind_t[map_kind_t["finish"] = 3] = "finish";
    map_kind_t[map_kind_t["path"] = 4] = "path";
})(map_kind_t || (map_kind_t = {}));
var map_item_t = (function () {
    function map_item_t(kind, lat, lon) {
        this.sector = -1;
        this.kind = map_kind_t.border;
        this.lat_f = 0;
        this.lon_f = 0;
        this.lat_f = lat;
        this.lon_f = lon;
    }
    Object.defineProperty(map_item_t.prototype, "lat", {
        get: function () {
            return this.lat_f;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(map_item_t.prototype, "lon", {
        get: function () {
            return this.lon_f;
        },
        enumerable: true,
        configurable: true
    });
    return map_item_t;
})();
var map_t = (function () {
    function map_t(id) {
        this._test_mode = false;
        this._id = "";
        this._cnv = undefined;
        this._canvas = undefined;
        this._ctx = undefined;
        this._is_init = false;
        this._maximaize = false;
        this._height = 1;
        this._width = 1;
        this._scale = 1;
        this._min_h = 1000000000;
        this._max_h = -1000000000;
        this._min_w = 1000000000;
        this._max_w = -1000000000;
        this._height_map = 1;
        this._width_map = 1;
        this._map_items = [];
        this._position_first = [];
        this._position_second = [];
        console.log("constructor, map_t, id: " + id);
        this._id = id;
        this._cnv = $("#" + id);
        this._canvas = this._cnv[0];
        var canvasSupported = !!document.createElement("canvas").getContext;
        if (canvasSupported) {
            this._ctx = this._canvas.getContext("2d");
            this._height = this._cnv.height();
            this._width = this._cnv.width();
            this._ctx.canvas.height = this._height;
            this._ctx.canvas.width = this._width;
            this.clear();
            this._is_init = true;
        }
        else {
            console.error("Can not get context");
            this._is_init = false;
            return;
        }
        this._cnv.click(function () {
            this._maximaize = !this._maximaize;
            if (this._maximaize) {
                $("#maxi_map").append($("#canvas_map"));
                $("#maxi_map").removeClass("dems-hide");
                $("#maxi_map").addClass("dems-show");
            }
            else {
                $("#mini_map").append($("#canvas_map"));
                $("#maxi_map").removeClass("dems-show");
                $("#maxi_map").addClass("dems-hide");
            }
            this.refresh();
        });
        Signal.bind("map", this.load_map, this);
        Signal.bind("add_position", this.add_position, this);
    }
    Object.defineProperty(map_t.prototype, "test_mode", {
        get: function () {
            return this._test_mode;
        },
        set: function (v) {
            this._test_mode = v;
            this.refresh();
        },
        enumerable: true,
        configurable: true
    });
    map_t.prototype.debug_draw = function () {
        console.log("map, debug_draw");
        this._ctx.font = "15px Courier";
        this._ctx.fillStyle = 'blue';
        this._ctx.fillText(this._canvas.height + " * " + this._canvas.width, 10, 20);
        this._ctx.fillText(this._cnv.height() + " * " + this._cnv.width(), 10, 40);
        this._ctx.fillText(this._height + " * " + this._width, 10, 60);
        this._ctx.fillText(this._scale, 10, 80);
        this._ctx.fillText(this._min_h + " * " + this._min_w, 10, 100);
        this._ctx.fillText(this._max_h + " * " + this._max_w, 10, 120);
        this._ctx.fillText(this._height_map + " * " + this._width_map, 10, 140);
    };
    map_t.prototype.test_draw = function () {
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
        this._ctx.strokeStyle = "blue";
    };
    map_t.prototype.load_map = function (data) {
        console.log("map, load_map, size: " + data.length);
        this._map_items = [];
        for (var i = 0; i < data.length; i++) {
            var kind = data[i].PAR[0].KND;
            var lon_s = data[i].PAR[3].LAF;
            lon_s = lon_s.replace(/\,/, ".");
            var lon = parseFloat(lon_s);
            var lat_s = data[i].PAR[4].LOF;
            lat_s = lat_s.replace(/\,/, ".");
            var lat = parseFloat(lat_s);
            var map_item = new map_item_t(kind, lat, lon);
            this._map_items.push(map_item);
        }
        this.set_bounds();
        this.set_scale();
        this.refresh();
    };
    map_t.prototype.set_bounds = function () {
        console.log("map, set_bounds");
        for (var i = 0; i < this._map_items.length; i++) {
            var lat = this._map_items[i].lat;
            var lon = this._map_items[i].lon;
            if ((lat == 0) && (lon == 0))
                continue;
            if (lat > this._max_h)
                this._max_h = lat;
            if (lat < this._min_h)
                this._min_h = lat;
            if (lon > this._max_w)
                this._max_w = lon;
            if (lon < this._min_w)
                this._min_w = lon;
        }
        this._height_map = this._max_h - this._min_h;
        this._width_map = this._max_w - this._min_w;
    };
    map_t.prototype.set_scale = function () {
        console.log("map, set_scale");
        var tmp_scale_h = this._height / this._height_map;
        var tmp_scale_w = this._width / this._width_map;
        if (tmp_scale_h < tmp_scale_w)
            this._scale = tmp_scale_h;
        else
            this._scale = tmp_scale_w;
    };
    map_t.prototype.add_position = function (data) {
        var lon_s = data[6].LAT;
        lon_s = lon_s.replace(/\,/, ".");
        var lon = parseFloat(lon_s);
        var lat_s = data[7].LON;
        lat_s = lat_s.replace(/\,/, ".");
        var lat = parseFloat(lat_s);
        var active = parseInt(data[20].ACT);
        if (active == active_t.first) {
            this._position_first = [];
            var map_item = new map_item_t("p", lat, lon);
            this._position_first.push(map_item);
        }
        else if (active = active_t.second) {
            this._position_second = [];
            var map_item = new map_item_t("p", lat, lon);
            this._position_second.push(map_item);
        }
        this.refresh();
    };
    map_t.prototype.clear = function () {
        if (!this._is_init)
            return;
        this._canvas.width = this._canvas.width;
    };
    map_t.prototype.begin_draw = function () {
        // console.log("map, begin_draw");
        this.clear();
    };
    map_t.prototype.end_draw = function () {
        // console.log("map, end_draw");
        this._ctx.stroke();
        this._ctx.fill();
    };
    map_t.prototype.draw_map = function () {
        // console.log("map, draw_map, count: " + this._map_items.length);
        this._ctx.strokeStyle = "Blue";
        this._ctx.beginPath;
        for (var i = 0; i < this._map_items.length; i++) {
            var lat = (this._map_items[i].lat - this._min_h) * this._scale;
            var lon = (this._map_items[i].lon - this._min_w) * this._scale;
            this._ctx.moveTo(lon, lat);
            this._ctx.arc(lon, lat, 1, 0, 2 * Math.PI);
        }
        this._ctx.closePath();
        this._ctx.stroke();
    };
    map_t.prototype.draw_client = function (client) {
        // console.log("map, draw_client, count: " + this._position_first.length);
        var list = [];
        if (client == active_t.first) {
            list = this._position_first;
            this._ctx.strokeStyle = "Red";
            this._ctx.fillStyle = "Red";
        }
        else if (client == active_t.second) {
            list = this._position_second;
            this._ctx.strokeStyle = "Green";
            this._ctx.fillStyle = "Green";
        }
        else
            return;
        this._ctx.beginPath;
        for (var i = 0; i < list.length; i++) {
            var lat = (list[i].lat - this._min_h) * this._scale;
            var lon = (list[i].lon - this._min_w) * this._scale;
            this._ctx.moveTo(lon, lat);
            this._ctx.arc(lon, lat, 5, 0, 2 * Math.PI);
        }
        this._ctx.closePath();
        this._ctx.stroke();
    };
    map_t.prototype.refresh = function () {
        if (!this._is_init)
            return;
        this.begin_draw();
        this.draw_map();
        this.draw_client(active_t.first);
        this.draw_client(active_t.second);
    };
    return map_t;
})();
var Signal = (function () {
    function Signal() {
    }
    Signal.bind = function (signal, method, context) {
        console.log("bind: " + signal);
        var tmp = {
            signal: signal,
            method: method
        };
        if (context)
            tmp.context = context;
        this.signals.push(tmp);
    };
    Signal.emit = function (signal, data) {
        // console.log("emit: " + signal);
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
        this._socket = undefined;
        this._host = "";
        this._is_connected = false;
        this._session_id = "";
        this._counter = 0;
        this._connection_status = undefined;
        this._last_recv_cmd = undefined;
        this._last_send_cmd = undefined;
        this._data_counter = undefined;
        console.log("constructor, web_socket_t");
        this._host = host;
        this._is_connected = false;
        this._connection_status = $("#connection_status");
        this._last_recv_cmd = $("#last_recv_cmd");
        this._last_send_cmd = $("#last_send_cmd");
        this._data_counter = $("#data_counter");
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
        message.splice(1, 0, ["par", this.session_id]);
        this._last_send_cmd.text("Send: " + message);
        message = JSON.stringify(message);
        this._socket.send(message);
    };
    web_socket_t.prototype.onOpen = function (evt) {
        console.log("onOpen");
        this._is_connected = true;
        this.session_id = String(new Date().getTime());
        this._connection_status.removeClass("label-danger");
        this._connection_status.removeClass("label-warning");
        this._connection_status.addClass("label-success");
        this._connection_status.text("Connection: open");
        Signal.emit("doSend", [["cmd", "ws_register"]]);
    };
    web_socket_t.prototype.onClose = function (evt) {
        console.log("onClose");
        this._is_connected = false;
        this._connection_status.removeClass("label-danger");
        this._connection_status.removeClass("label-success");
        this._connection_status.addClass("label-warning");
        this._connection_status.text("Connection: close");
    };
    web_socket_t.prototype.onError = function (evt) {
        console.log(evt.data);
        this._is_connected = false;
        this._connection_status.removeClass("label-success");
        this._connection_status.removeClass("label-warning");
        this._connection_status.addClass("label-danger");
        this._connection_status.text("Connection: error");
    };
    web_socket_t.prototype.onMessage = function (evt) {
        var data;
        data = JSON.parse(evt.data);
        if (data.length == 0)
            return;
        if (data[0].CMD != undefined) {
            var cmd = data[0].CMD;
            this._last_recv_cmd.text("Recv: " + cmd);
            data.shift();
            Signal.emit(cmd, data);
        }
        else {
            this._counter++;
            this._data_counter.text("Counter: " + this._counter);
            Signal.emit("add_data", data);
        }
    };
    Object.defineProperty(web_socket_t.prototype, "is_connected", {
        get: function () {
            return this._is_connected;
        },
        set: function (v) {
            this._is_connected = v;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(web_socket_t.prototype, "session_id", {
        get: function () {
            return this._session_id;
        },
        set: function (v) {
            this._session_id = v;
        },
        enumerable: true,
        configurable: true
    });
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
    var arr = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, "A", "B", "C", "D", "E", "F"];
    return arr[value >> 4] + "" + arr[value & 0xF];
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
    Object.defineProperty(custom_t.prototype, "self", {
        get: function () {
            return this._self;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(custom_t.prototype, "owner", {
        get: function () {
            return this._owner;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(custom_t.prototype, "ext_id", {
        get: function () {
            return this._ext_id;
        },
        set: function (v) {
            this._ext_id = v;
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
    function row_t(id, owner, ext_id) {
        _super.call(this, id, "<tr/>", owner);
        this._cells = [];
        this.ext_id = ext_id;
        var self = this;
        this._self.click(function () {
            $.get("command?cmd=ws_activate&par=" + self.ext_id + "&par=next", function (data) {
            });
        });
    }
    Object.defineProperty(row_t.prototype, "cells", {
        get: function () {
            return this._cells;
        },
        enumerable: true,
        configurable: true
    });
    row_t.prototype.find_by_id = function (id) {
        for (var i = 0; i < this._cells.length; i++) {
            if (this._cells[i].id == id)
                return this._cells[i];
        }
        return undefined;
    };
    row_t.prototype.add_cell = function (id, text) {
        var cell = this.find_by_id(id);
        if (cell == undefined) {
            cell = new cell_t(id, this._self);
            this._cells.push(cell);
        }
        cell.text = text;
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
    table_t.prototype.find_row = function (id) {
        for (var i = 0; i < this._rows.length; i++) {
            if (this._rows[i].id == id)
                return this._rows[i];
        }
        return undefined;
    };
    table_t.prototype.do_add_row = function (id, ext_id) {
        var row = this.find_row(id);
        if (row == undefined) {
            row = new row_t(id, this._self, ext_id);
            this._rows.push(row);
        }
        return row;
    };
    return table_t;
})(custom_t);
var clients_table_t = (function (_super) {
    __extends(clients_table_t, _super);
    function clients_table_t(id, cols_count) {
        console.log("constructor, clients_table_t, id: " + id);
        _super.call(this, id, $(window), cols_count);
    }
    clients_table_t.prototype.get_id = function (id) {
        return "client_" + String(id);
    };
    clients_table_t.prototype.add_row = function (id, name, ext_id) {
        var row_id = this.get_id(id);
        var row = _super.prototype.do_add_row.call(this, row_id, ext_id);
        var cell_id = "client_name_" + String(id);
        row.add_cell(cell_id, name);
        cell_id = "client_act_" + String(id);
        row.add_cell(cell_id, "");
        cell_id = "client_id_" + String(id);
        row.add_cell(cell_id, String(id));
        return row;
    };
    clients_table_t.prototype.active_row = function (row, active) {
        switch (active) {
            case active_t.first: {
                row.self.removeClass("dems-second");
                row.self.addClass("dems-first").siblings().removeClass("dems-first");
                break;
            }
            case active_t.second: {
                row.self.removeClass("dems-first");
                row.self.addClass("dems-second").siblings().removeClass("dems-second");
                break;
            }
        }
    };
    clients_table_t.prototype.state_row = function (row, state) {
        if (state == state_t.start)
            row.self.addClass("dems-active").siblings().removeClass("dems-active");
        else
            row.self.removeClass("dems-active");
    };
    clients_table_t.prototype.register_row = function (row, register) {
        if (register == register_t.ok)
            row.self.addClass("dems-register").siblings().removeClass("dems-register");
        else
            row.self.removeClass("dems-register");
    };
    clients_table_t.prototype.add_client = function (client) {
        return this.add_row(client.id, client.name, String(client.id));
    };
    clients_table_t.prototype.active_client = function (client, active) {
        var id = this.get_id(client.id);
        var row = this.find_row(id);
        this.active_row(row, active);
        var cell_id = "client_act_" + String(client.id);
        element.set_text(cell_id, active_t[active]);
    };
    clients_table_t.prototype.state_client = function (client, state) {
        var id = this.get_id(client.id);
        var row = this.find_row(id);
        this.state_row(row, state);
    };
    clients_table_t.prototype.register_client = function (client, register) {
        var id = this.get_id(client.id);
        var row = this.find_row(id);
        this.register_row(row, register);
    };
    return clients_table_t;
})(table_t);
var data_table_t = (function (_super) {
    __extends(data_table_t, _super);
    function data_table_t(id, cols_count) {
        console.log("constructor, data_table_t, id: " + id);
        _super.call(this, id, $(window), cols_count);
    }
    data_table_t.prototype.add_row = function (prefix, key, value) {
        var row_id = "data_" + prefix + "_" + key + "_" + value;
        var row = _super.prototype.find_row.call(this, row_id);
        if (row == undefined) {
            row = _super.prototype.do_add_row.call(this, row_id, "");
            var cell_id_1 = "data_" + prefix + "_key_" + key;
            row.add_cell(cell_id_1, key);
        }
        var cell_id = "data_" + prefix + "_value_" + key;
        row.add_cell(cell_id, value);
    };
    return data_table_t;
})(table_t);
//# sourceMappingURL=script.js.map