//==============================================================================
//==============================================================================
enum active_t {
  none = 0,
  first,
  second,
  next
};
//==============================================================================
enum state_t {
  none = 0,
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
  private _name    : string             = "unnamed";
  private _state   : state_t            = state_t.none;
  private _active  : active_t           = active_t.none;
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
  public get state() : state_t {
    return this._state;
  }
  //----------------------------------------------------------------------------
  public set state(v : state_t) {
    this._state = v;
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
  private _clients_table: clients_table_t;
  private _data_first_table: data_table_t;
  private _data_second_table: data_table_t;
  //----------------------------------------------------------------------------
  constructor() {
    this._clients_table     = new clients_table_t("remote_clients",     2);
    this._data_first_table  = new data_table_t   ("remote_data_first",  2);
    this._data_second_table = new data_table_t   ("remote_data_second", 2);

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
  private add_client(data: any): void {
    let id      : number     = data[0]._ID;
    let name    : string     = data[1].NAM;
    let state   : state_t    = parseInt(data[2].STA);
    let active  : active_t   = parseInt(data[3].ACT);
    let register: register_t = parseInt(data[4].REG);

    let client: client_t = this.get_client_by_id(id);
    if(client == undefined){
      client = new client_t(id, name);

      this._clients.push(client);
      this._clients_table.add_client(client);
    }

    this.switch_current(active, client);

    client.state    = state;
    client.active   = active;
    client.register = register;

    this._clients_table.state_client   (client, state);
    this._clients_table.active_client  (client, active);
    this._clients_table.register_client(client, register);
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
    let active: active_t = active_t.none;
    for(let i = 0; i < data.length; i++){
      if(data[i].ACT != undefined){
        active = data[i].ACT;
        break;
      }
    }

    let id: string = '';
    for(let i = 0; i < data.length; i++){
      if(data[i]._ID != undefined){
        id = data[i]._ID;
        break;
      }
    }

    let data_table: data_table_t = undefined;
    let prefix: string;
    if(active == active_t.first){
      data_table = this._data_first_table;
      prefix = "first";
    }
    else{
      data_table = this._data_second_table;
      prefix = "second";
    }

    for (var i = 0; i < data.length; i++){
      if(static_filter.indexOf(Object.keys(data[i])[0]) != -1){
        let param = Object.keys(data[i])[0];
        let value = data[i][param];

        data_table.add_row(prefix, param, value);
      }
    }

    Signal.emit("add_position", data);
  }
  //----------------------------------------------------------------------------
  private switch_current(current: active_t, client: client_t): void{
    let photo: string = "/pilots/" + client.name + '.jpg';
    let info: string  = "/pilots/" + client.name + '_info.html';

    if(current == active_t.first)
    {
      element.set_src("first_pilot_photo", photo);
      element.set_src("first_pilot_info",  info);
    }
    else if(current == active_t.second)
    {
      element.set_src("second_pilot_photo", photo);
      element.set_src("second_pilot_info",  info);
    }
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
