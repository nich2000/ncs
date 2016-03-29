//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class custom_t {
  private _id: string;
  private _owner: any;

  constructor(id: string, owner: any) {
    this._id = id;
    this._owner = owner;
  }

  public get id() : string {
    return this._id;
  }
}
//==============================================================================
class cell_t extends custom_t {
  private _cell: any;

  constructor(id: string, owner: any) {
    super(id, owner);

    this._cell = $("<tr></tr>");
    this._cell.attr("id", this.id);
  }

  public get text() : string {
    return this._cell.text();
  }

  public set text(v : string) {
    this._cell.text(v);
  }
}
//==============================================================================
class row_t extends custom_t {
  private _row: any;
  private _cells: cell_t[];

  constructor(id: string, owner: any) {
    super(id, owner);

    this._row = $("<tr></tr>");
    this._row.attr("id", this.id);

    this._row.click(function() {
      $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
      let cell: any = $(this).find('td:last');
      let value = cell.text();
      Signal.emit("doSend", [["cmd", "activate"], ["par", value], ["par", "on"]]);
    });
  }

  public get cells() : cell_t[] {
    return this._cells;
  }

  public add_cell(id: string, text: string) {
  //   if (!this.exists_cell(id)) {
  //     cell = $("<td style='height:8px'></td>");
  //     cell.attr("id", cell_id);
  //     line.append(cell);
  //   }
  //   cell.text();
  }

  public exists_cell(id: string): boolean {
    return $("#" + id).length > 0;
  }
}
//==============================================================================
class table_t extends custom_t {
  private _table: any;
  private _cols_count: number;
  private _rows: row_t[];

  constructor(id: string, owner: any, cols_count: number) {
    console.log("constructor: table_t, id: " + id);

    super(id, owner);

    this._table = $("#" + this.id);

    this._cols_count = cols_count;
  }

  public get rows() : row_t[] {
    return this._rows;
  }

  public add_row(data: any) {

  }

  public exists_row(id: string): boolean {
    return $("#" + this.id).length > 0;
  }
}
//==============================================================================
class clients_table_t extends table_t {
  constructor(id: string, owner: any, cols_count: number) {
    super(id, owner, cols_count);

    Signal.bind("add_client", this.add_client, this);
  }

  add_client(data: any) {
    if (data.PAR == undefined)
      return;

    this.add_row(data.PAR);
  }

  add_row(data: any) {
    let client: client_t = new client_t(data[0].PAR, data[1].PAR);

    let row_id: string = "client_" + client.id + "_" + client.name;

    if(!this.exists_row(row_id)) {
      // let row: row_t = new row_t(row_id, this.table);
      // this.rows.push(row);

      // this.table.append(row.row);

      // let cell_id: string = "name_" + row_id;
      // row.add_cell(cell_id, name);

      // cell_id = "id_" + row_id;
      // row.add_cell(cell_id, id);
    }
  }
}
//==============================================================================
class data_table_t extends table_t {
  constructor(id: string, owner: any, cols_count: number) {
    super(id, owner, cols_count);

    Signal.bind("add_data", this.add_row, this);
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
