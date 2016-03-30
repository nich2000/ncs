//==============================================================================
enum state_t {
  state_none,
  state_start,
  state_starting,
  state_stop,
  state_stopping,
  state_pause,
  state_pausing,
  state_resume,
  state_resuming
}
//==============================================================================
class client_t {
  private _id: number;
  private _name: string;
  private _state: state_t;

  public constructor(id: number, name: string) {
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
}
//==============================================================================
class clients_t {
  private _clients: client_t[];
  private _table: clients_table_t;

  constructor() {
    this._table = new clients_table_t("remote_clients", 2);

    Signal.bind("add_client", this.add_client, this);
  }

  public get items() : client_t[] {
    return this._clients;
  }

  public add_client(data: any) {
    let id = data[0].PAR;
    let name = data[1].PAR;

    if(!this.exists_by_id(id)){
      let client = new client_t(id, name);
      this._clients.push(client);

      this._table.add_row(id, name);
    }
  }

  public get_by_id(id: number) {
  }

  public exists_by_id(id: number) {
  }
}
//==============================================================================
