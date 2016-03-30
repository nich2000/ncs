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

  public get id() : string {
    return this._id;
  }
}
//==============================================================================
class cell_t extends custom_t {

  constructor(id: string, owner: row_t) {
    super(id, "<td/>", owner);

    // this._self = element.add(this.id, "<td/>", owner._self);
  }

  // public get text() : string {
    // return this._cell.text();
  // }

  // public set text(v : string) {
    // this._cell.text(v);
  // }
}
//==============================================================================
class row_t extends custom_t {
  private _cells: cell_t[];

  constructor(id: string, owner: any) {
    super(id, "<tr/>", owner);

    this._self.click(function() {
      $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
      let cell: any = $(this).find('td:last');
      let value: string = cell.text();
      Signal.emit("doSend", [["cmd", "activate"], ["par", value], ["par", "on"]]);
    });
  }

  public get cells() : cell_t[] {
    return this._cells;
  }

  public add_cell(id: string, text: string) {
  }
}
//==============================================================================
class table_t extends custom_t {
  protected _cols_count: number;
  protected _rows: row_t[];

  constructor(id: string, owner: any, cols_count: number) {
    console.log("constructor: table_t, id: " + id);

    super(id, "<table/>", owner);

    this._cols_count = cols_count;
  }

  public get rows() : row_t[] {
    return this._rows;
  }
}
//==============================================================================
class clients_table_t extends table_t {

  constructor(id: string, cols_count: number) {
    super(id, $(window), cols_count);
  }

  add_row(id: number, name: string) {

  }
}
//==============================================================================
class data_table_t extends table_t {

  constructor(id: string, cols_count: number) {
    super(id, $(window), cols_count);
    // Signal.bind("add_data", this.add_row, this);
  }

  add_row(data: any) {
    // var line_id = data[0];
    // var line = $("#" + line_id);
    // if (line.length == 0) {
    //   line = $("<tr></tr>");
    //   line.attr("id", line_id);
    //   this.table.append(line);
    // }

    // var cell_id = "key_" + data[0];
    // var cell = $("#" + cell_id);
    // if (cell.length == 0) {
    //   cell = $("<td style='height:8px'></td>");
    //   cell.attr("id", cell_id);
    //   line.append(cell);
    // }
    // cell.text(data[1]);

    // cell_id = "value_" + data[0];
    // cell = $("#" + cell_id);
    // if (cell.length == 0) {
    //   cell = $("<td style='height:8px'></td>");
    //   cell.attr("id", cell_id);
    //   line.append(cell);
    // }
    // cell.text(data[2]);
  }
}
//==============================================================================
