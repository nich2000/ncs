//==============================================================================
//==============================================================================
enum connect_t {
  unknown = 0,
  connected,
  disconnected
}
//==============================================================================
enum active_t {
  next = -1,
  unknown,
  first,
  second
};
//==============================================================================
enum state_t {
  unknown = 0,
  start,
  starting,
  stop,
  stopping,
  pause,
  pausing,
  resume,
  resuming,
  step
}
//==============================================================================
enum register_t {
  none = 0,
  ok
};
//==============================================================================
let static_filter: string[] = [
  "_ID",                 // 1
  "TIM",                 // 2
  "T_S",                 // 3
  "CNT",                 // 4
  "SPD",                 // 5
  "HEA",                 // 6
  "LAT",                 // 7
  "LON"                  // 8
  // "UN1",                 // 9
  // "UN2",                 // 10
  // "AZ1",                 // 11
  // "AZ2",                 // 12
  // "MT1",                 // 13
  // "MT2",                 // 14
  // "BVL",                 // 15
  // "UN3",                 // 16
  // "UN4",                 // 17
  // "EVL",                 // 18
  // "USB"                  // 19
];
//==============================================================================
interface data_i {
  key  : string;
  value: string;
}
//==============================================================================
interface data_list_i {
  items: Array<data_i>;
}
//==============================================================================
class client_t {
  //----------------------------------------------------------------------------
  private _id      : number             = -1;
  private _name    : string             = "noname";
  private _session : string             = "nosession";
  private _state   : state_t            = state_t.unknown;
  private _connect : connect_t          = connect_t.unknown;
  private _active  : active_t           = active_t.unknown;
  private _register: register_t         = register_t.none;
  private _data    : Array<data_list_i> = [];
  //----------------------------------------------------------------------------
  constructor(id: number, name: string) {
    this._id = id;
    this._name = name;
  }
  //----------------------------------------------------------------------------
  public get id() : number {
    return this._id;
  }
  //----------------------------------------------------------------------------
  public get name() : string {
    return this._name;
  }
  //----------------------------------------------------------------------------
  public set name(v : string) {
    this._name = v;
  }
  //----------------------------------------------------------------------------
  public get session() : string {
    return this._session;
  }
  //----------------------------------------------------------------------------
  public set session(v : string) {
    this._session = v;
  }
  //----------------------------------------------------------------------------
  public get state() : state_t {
    return this._state;
  }
  //----------------------------------------------------------------------------
  public set state(v : state_t) {
    this._state = v;
  }
  //----------------------------------------------------------------------------
  public get connect() : connect_t {
    return this._connect;
  }
  //----------------------------------------------------------------------------
  public set connect(v : connect_t) {
    this._connect = v;
  }
  //----------------------------------------------------------------------------
  public get active() : active_t {
    return this._active
  }
  //----------------------------------------------------------------------------
  public set active(v : active_t) {
    this._active = v;
  }
  //----------------------------------------------------------------------------
  public get register() : register_t {
    return this._register;
  }
  //----------------------------------------------------------------------------
  public set register(v : register_t) {
    this._register = v;
  }
  //----------------------------------------------------------------------------
  private exists_data_key(key: string): boolean{
    // for(let i: number; i < this._data.length; i++)
    //   if(this._data[i].key == key)
    //     return true;
    return false;
  }
  //----------------------------------------------------------------------------
  private get_data_by_key(key: string): data_i{
    // for(let i = 0; i < this._data.length; i++)
    //   if(this._data[i].key == key)
    //     return this._data[i];
    return undefined;
  }
  //----------------------------------------------------------------------------
  public add_data(data: any): void{
    // let d: data_i = this.get_data_by_key(data[0]);
    // if(d == undefined){
    //   d = {
    //     key: data[0],
    //     value: data[1]
    //   }
    //   this._data.push(d);
    // }
    // else{
    //   d.value = data[1];
    // }
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class clients_t {
  //----------------------------------------------------------------------------
  private _clients: Array<client_t> = [];
  //----------------------------------------------------------------------------
  private _clients_table:     clients_table_t = undefined;
  private _data_first_table:  data_table_t    = undefined;
  private _data_second_table: data_table_t    = undefined;
  //----------------------------------------------------------------------------
  constructor() {
    console.log("constructor, clients_t");

    try {
      this._clients_table     = new clients_table_t("remote_clients",     3);
    }
    catch(e) {
    }

    try {
      this._data_first_table  = new data_table_t   ("remote_data_first",  2);
    }
    catch(e) {
    }

    try {
      this._data_second_table = new data_table_t   ("remote_data_second", 2);
    }
    catch(e){
    }

    Signal.bind("clients",  this.refresh_clients, this);
    Signal.bind("add_data", this.add_data,        this);
  }
  //----------------------------------------------------------------------------
  public get items(): client_t[] {
    return this._clients;
  }
  //----------------------------------------------------------------------------
  private refresh_clients(data: any): void {
    for(let i = 0; i < data.length; i++){
      this.add_client(data[i].PAR);
    }
  }
  //----------------------------------------------------------------------------
  private add_client(data: any): client_t {
    let id      : number     =          data[0]._ID;
    let name    : string     =          data[1].NAM;
    let session : string     =          data[2].SES;
    let state   : state_t    = parseInt(data[3].STA);
    let connect : connect_t  = parseInt(data[4].CON);
    let active  : active_t   = parseInt(data[5].ACT);
    let register: register_t = parseInt(data[6].REG);

    let client: client_t = this.get_client_by_id(id);
    if(client == undefined){
      client = new client_t(id, name);

      this._clients.push(client);
      let row: row_t = this._clients_table.add_client(client);
      // row.onclick = this.select_client;
    }

    client.session  = session;
    client.connect  = connect;
    client.state    = state;
    client.active   = active;
    client.register = register;

    this.switch_current(active, client);

    this._clients_table.state_client   (client, state);
    this._clients_table.active_client  (client, active);
    this._clients_table.register_client(client, register);

    return client;
  }
  //----------------------------------------------------------------------------
  private get_client_by_id(id: number): client_t {
    for (let i = 0; i < this._clients.length; i++)
      if (this._clients[i].id == id)
        return this._clients[i];
    return undefined;
  }
  //----------------------------------------------------------------------------
  private get_client_by_name(name: string): client_t {
    for (let i = 0; i < this._clients.length; i++)
      if (this._clients[i].name == name)
        return this._clients[i];
    return undefined;
  }
  //----------------------------------------------------------------------------
  private exists_client_by_id(id: number): boolean {
    for (let i = 0; i < this._clients.length; i++)
      if (this._clients[i].id == id)
        return true;
    return false;
  }
  //----------------------------------------------------------------------------
  private add_data(data: any): void {
    let active: active_t = active_t.unknown;
    for(let i = data.length-1; i >= 0; i--){
      if(data[i].ACT != undefined){
        active = data[i].ACT;
        break;
      }
    }

    let data_table: data_table_t = undefined;
    let prefix: string;
    if(active == active_t.first){
      data_table = this._data_first_table;
      prefix = "first";
    }
    else if(active == active_t.second){
      data_table = this._data_second_table;
      prefix = "second";
    }
    else
      return;

    try{
      let begin: any = new Date();
      Signal.emit("add_position", data);
      let end: any = new Date;
      profiler.text(end - begin);
    }
    catch(e){
    }

    if(data_table == undefined)
      return;

    let id: string = "";
    for(let i = 0; i < data.length; i++){
      if(data[i]._ID != undefined){
        id = data[i]._ID;
        break;
      }
    }

    for (var i = 0; i < data.length; i++){
      if(static_filter.indexOf(Object.keys(data[i])[0]) != -1){
        let param = Object.keys(data[i])[0];
        let value = data[i][param];

        // let b: any = new Date();
        data_table.add_row(prefix, param, value);
        // let e: any = new Date;
        // profiler.text(e - b);
      }
    }
  }
  //----------------------------------------------------------------------------
  private switch_current(current: active_t, client: client_t): void{
    let photo: string = "/pilots/" + client.name + ".jpg";
    let info: string  = "/pilots/" + client.name + "_info.html";

    if(current == active_t.first) {
      element.set_src("first_pilot_photo", photo);
      element.set_src("first_pilot_info",  info);
    }
    else if(current == active_t.second) {
      element.set_src("second_pilot_photo", photo);
      element.set_src("second_pilot_info",  info);
    }
  }
  //----------------------------------------------------------------------------
  private select_client(id: string): void {
    console.log("select_client: " + id);

    let client: client_t = this.get_client_by_id(parseInt(id));

    $.get("command?cmd=ws_activate&par=" + client.name + "&par=" + client.id + "&par=next",
      function(data) {
      }
    );
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
