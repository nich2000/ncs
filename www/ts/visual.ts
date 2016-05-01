//==============================================================================
//==============================================================================
class custom_t {
  //----------------------------------------------------------------------------
  protected _id: string;
  protected _owner: any;
  protected _self: any;
  //----------------------------------------------------------------------------
  constructor(id: string, tag: string, owner: any) {
    this._id = id;
    this._owner = owner;

    this._self = element.find_by_id(id);
    if(this._self == undefined)
      this._self = element.add(id, tag, owner);
  }
  //----------------------------------------------------------------------------
  public get id(): string {
    return this._id;
  }
  //----------------------------------------------------------------------------
  public get self() : any {
    return this._self;
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class cell_t extends custom_t {
  //----------------------------------------------------------------------------
  private _text: string;
  //----------------------------------------------------------------------------
  constructor(id: string, owner: row_t) {
    super(id, "<td/>", owner);
  }
  //----------------------------------------------------------------------------
  public get text() : string {
    return this._self.text();
  }
  //----------------------------------------------------------------------------
  public set text(v : string) {
    this._self.text(v);
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class row_t extends custom_t {
  //----------------------------------------------------------------------------
  private _cells: Array<cell_t> = [];
  //----------------------------------------------------------------------------
  constructor(id: string, owner: any, onclick: Function) {
    super(id, "<tr/>", owner);

    if(onclick != undefined)
      this._self.click(onclick);

    // #govnocode
    // this._self.click(function() {
    //   // let cell: any = $(this).find("td:last");
    //   let cell: any = $(this).find("td:first");
    //   // Signal.emit("doSend", [["cmd", "ws_activate"], ["par", cell.text()], ["par", "next"]]);
    //   $.get("command?cmd=ws_activate&par=" + cell.text() + "&par=next",
    //     function(data) {
    //     }
    //   );
    // });
  }
  //----------------------------------------------------------------------------
  public get cells(): cell_t[] {
    return this._cells;
  }
  //----------------------------------------------------------------------------
  private find_by_id(id: string): cell_t {
    for(let i = 0; i < this._cells.length; i++){
      if(this._cells[i].id == id)
        return this._cells[i];
    }

    return undefined;
  }
  //----------------------------------------------------------------------------
  public add_cell(id: string, text: string): void {
    let cell: cell_t = this.find_by_id(id);

    if(cell == undefined){
      cell = new cell_t(id, this._self);
      this._cells.push(cell);
    }

    cell.text = text;
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class table_t extends custom_t {
  //----------------------------------------------------------------------------
  private _onrowclick: Function;
  //----------------------------------------------------------------------------
  protected _cols_count: number;
  protected _rows: Array<row_t> = [];
  //----------------------------------------------------------------------------
  constructor(id: string, owner: any, cols_count: number, onrowclick: Function) {
    super(id, "<table/>", owner);

    this._cols_count = cols_count;
    this._onrowclick = onrowclick;
  }
  //----------------------------------------------------------------------------
  public get rows(): row_t[] {
    return this._rows;
  }
  //----------------------------------------------------------------------------
  protected find_row(id: string): row_t {
    for(let i: number = 0; i < this._rows.length; i++){
      if(this._rows[i].id == id)
        return this._rows[i];
    }

    return undefined;
  }
  //----------------------------------------------------------------------------
  protected do_add_row(id: string): row_t {
    let row: row_t = this.find_row(id);
    if (row == undefined) {
      row = new row_t(id, this._self, this._onrowclick);
      this._rows.push(row);
    }

    return row;
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class clients_table_t extends table_t {
  //----------------------------------------------------------------------------
  constructor(id: string, cols_count: number, onrowclick: Function) {
    super(id, $(window), cols_count, onrowclick);
  }
  //----------------------------------------------------------------------------
  private get_id(id: number): string {
    return "client_" + String(id);
  }
  //----------------------------------------------------------------------------
  private add_row(id: number, name: string): void {
    let row_id: string = this.get_id(id);
    let row: row_t = super.do_add_row(row_id);

    let cell_id: string = "client_name_" + String(id);
    row.add_cell(cell_id, name);

    cell_id = "client_act_" + String(id);
    row.add_cell(cell_id, "");

    cell_id = "client_id_" + String(id);
    row.add_cell(cell_id, String(id));
  }
  //----------------------------------------------------------------------------
  private active_row(row: row_t, active: active_t): void {
    switch(active){
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
  }
  //----------------------------------------------------------------------------
  private state_row(row: row_t, state: state_t): void {
    if(state == state_t.start)
      row.self.addClass("dems-active").siblings().removeClass("dems-active");
    else
      row.self.removeClass("dems-active");
  }
  //----------------------------------------------------------------------------
  private register_row(row: row_t, register: register_t): void {
    if(register == register_t.ok)
      row.self.addClass("dems-register").siblings().removeClass("dems-register");
    else
      row.self.removeClass("dems-register");
  }
  //----------------------------------------------------------------------------
  public add_client(client: client_t): void {
    this.add_row(client.id, client.name);
  }
  //----------------------------------------------------------------------------
  public active_client(client: client_t, active: active_t): void {
    let id: string = this.get_id(client.id);
    let row: row_t = this.find_row(id);

    this.active_row(row, active);

    let cell_id: string = "client_act_" + String(client.id);
    element.set_text(cell_id, active_t[active]);
  }
  //----------------------------------------------------------------------------
  public state_client(client: client_t, state: state_t): void {
    let id: string = this.get_id(client.id);
    let row: row_t = this.find_row(id);

    this.state_row(row, state);
  }
  //----------------------------------------------------------------------------
  public register_client(client: client_t, register: register_t): void {
    let id: string = this.get_id(client.id);
    let row: row_t = this.find_row(id);

    this.register_row(row, register);
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class data_table_t extends table_t {
  //----------------------------------------------------------------------------
  constructor(id: string, cols_count: number, onrowclick: Function) {
    super(id, $(window), cols_count, onrowclick);
  }
  //----------------------------------------------------------------------------
  add_row(prefix: string, key: string, value: string) : void {
    let row_id: string = "data_" + prefix + "_" + key + "_" + value;

    let row: row_t = super.find_row(row_id)
    if(row == undefined)
    {
      super.do_add_row(row_id);

      let cell_id: string = "data_" + prefix + "_key_" + key;
      row.add_cell(cell_id, key);
    }

    let cell_id = "data_" + prefix + "_value_" + key;
    row.add_cell(cell_id, value);
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
