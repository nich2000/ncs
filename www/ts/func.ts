//==============================================================================
enum current_t{
  none,
  first,
  second
}
//==============================================================================
enum state_t {
  none,
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
let static_filter: string[] = [
  "_ID",                 // 1
  "TIM",                 // 2
  // "T_S",                 // 3
  // "CNT",                 // 4
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
  key: string;
  value: string;
}
//==============================================================================
class client_t {
  private _id: number;
  private _name: string;
  private _state: state_t = state_t.none;
  private _data: Array<data_i> = [];

  constructor(id: number, name: string) {
    this._id = id;
    this._name = name;
  }

  public get id() : number {
    return this._id;
  }

  public get state() : state_t {
    return this._state;
  }

  public set state(v : state_t) {
    this._state = v;
  }

  public get name() : string {
    return this._name;
  }

  public set name(v : string) {
    this._name = v;
  }

  private exists_data_key(key: string): boolean{
    for(let i: number; i < this._data.length; i++)
      if(this._data[i].key == key)
        return true;
    return false;
  }

  private get_data_by_key(key: string): data_i{
    for(let i = 0; i < this._data.length; i++)
      if(this._data[i].key == key)
        return this._data[i];
    return undefined;
  }

  public add_data(data: any): void{
    let d: data_i = this.get_data_by_key(data[0]);
    if(d == undefined){
      d = {
        key: data[0],
        value: data[1]
      }
      this._data.push(d);
    }
    else{
      d.value = data[1];
    }
  }

}
//==============================================================================
class clients_t {
  private _clients: Array<client_t> = [];
  private _clients_table: clients_table_t;
  private _data_first_table: data_table_t;
  private _data_second_table: data_table_t;

  constructor() {
    this._clients_table = new clients_table_t("remote_clients", 2);
    this._data_first_table = new data_table_t("remote_data_first", 2);
    this._data_second_table = new data_table_t("remote_data_second", 2);

    Signal.bind("add_client", this.add_client, this);
    Signal.bind("add_data", this.add_data, this);
  }

  public get items(): client_t[] {
    return this._clients;
  }

  public add_client(data: any): void {
    let id: number = data[0].PAR;
    let name: string = data[1].PAR;

    if(!this.exists_by_id(id)){
      let client = new client_t(id, name);
      this._clients.push(client);

      this._clients_table.add_row(client.id, client.name);
    }
  }

  public get_by_id(id: number): client_t {
    for (let i = 0; i < this._clients.length; i++)
      if (this._clients[i].id == id)
        return this._clients[i];
    return undefined;
  }

  public exists_by_id(id: number): boolean {
    for (let i = 0; i < this._clients.length; i++)
      if (this._clients[i].id == id)
        return true;
    return false;
  }

  private add_data(data: any): void{
    let current: current_t = data[0][1];

    let client: client_t = this.get_by_id(data[1][1]);
    if(client == undefined)
      return;
    else
      this.switch_current(current, client);

    if(static_filter.indexOf(data[i][0]) != -1){
      for (var i = 2; i < data.length; i++){
        client.add_data(data);

        if(current == current_t.first)
          this._data_first_table.add_row('first', data[i][0], data[i][1]);
        else
          this._data_second_table.add_row('second', data[i][0], data[i][1]);
      }
    }
  }

  private switch_current(current: current_t, client: client_t): void{
  }
}
//==============================================================================
