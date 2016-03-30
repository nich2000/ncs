//==============================================================================
class custom_t {
  protected _id: string;
  protected _owner: any;
  protected _self: any;

  constructor(id: string, tag: string, owner: any) {
    this._id = id;
    this._owner = owner;

    this._self = element.find_by_id(id);
    if(this._self == undefined)
      this._self = element.add(id, tag, owner);
  }

  public get id(): string {
    return this._id;
  }
}
//==============================================================================
class cell_t extends custom_t {
  private _text: string;

  constructor(id: string, owner: row_t) {
    super(id, "<td/>", owner);
  }

  public get text() : string {
    return this._self.text();
  }

  public set text(v : string) {
    this._self.text(v);
  }
}
//==============================================================================
class row_t extends custom_t {
  private _cells: Array<cell_t> = [];

  constructor(id: string, owner: any) {
    super(id, "<tr/>", owner);

    this._self.click(function() {
      $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
      let cell: any = $(this).find('td:last');
      let value: string = cell.text();
      Signal.emit("doSend", [["cmd", "activate"], ["par", value], ["par", "on"]]);
    });
  }

  public get cells(): cell_t[] {
    return this._cells;
  }

  public add_cell(id: string, text: string): void {
    if(!element.exists_by_id(id)){
      let cell: cell_t = new cell_t(id, this._self);

      this._cells.push(cell);

      cell.text = text;
    }
  }
}
//==============================================================================
class table_t extends custom_t {
  protected _cols_count: number;
  protected _rows: Array<row_t> = [];

  constructor(id: string, owner: any, cols_count: number) {
    super(id, "<table/>", owner);

    this._cols_count = cols_count;
  }

  public get rows(): row_t[] {
    return this._rows;
  }

  protected do_add_row(id: string): row_t{
    if (!element.exists_by_id(id)) {
      let row: row_t = new row_t(id, this._self);

      this._rows.push(row);

      return row;
    }
  }
}
//==============================================================================
class clients_table_t extends table_t {
  constructor(id: string, cols_count: number) {
    super(id, $(window), cols_count);
  }

  add_row(id: number, name: string) {
    let row_id: string = 'client_' + String(id) + '_' + name;

    let row: row_t = super.do_add_row(row_id);

    let cell_id: string = 'client_name_' + String(id);
    row.add_cell(cell_id, name);

    cell_id = 'client_id_' + String(id);
    row.add_cell(cell_id, String(id));
  }
}
//==============================================================================
class data_table_t extends table_t {
  constructor(id: string, cols_count: number) {
    super(id, $(window), cols_count);
  }

  add_row(prefix: string, key: string, value: string) {
    let row_id: string = 'data_' + prefix + '_' + key + '_' + value;

    let row: row_t = super.do_add_row(row_id);

    let cell_id: string = 'data_' + prefix + '_key_' + key;
    row.add_cell(cell_id, key);

    cell_id = 'data_' + prefix + '_value_' + key;
    row.add_cell(cell_id, value);
  }
}
//==============================================================================
