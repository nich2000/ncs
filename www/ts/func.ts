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

class client_t {
	private _id: string;
	private _name: string;
	private _state: state_t;

  public constructor(id: string, name: string) {
    this._id = id;
    this._name = name;
  }

  public get id() : string {
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

class clients_t {

}
