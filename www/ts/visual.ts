//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class cell_t
{
  public owner: any;
  public cell: any;
  public id: string;

  public index: number;

  public text: string;
  public html: string;

  constructor(id: string, owner: any)
  {
    this.id = id;
    this.owner = owner;
    this.cell = $("#" + this.id);
  }
}
//==============================================================================
class row_t
{
  public owner: any;
  public row: any;
  public id: string;

  public index: number;
  public count: number;

  public new_row: boolean;

  public cells: cell_t[];

  constructor(id: string, owner: any)
  {
    this.id = id;
    this.owner = owner;
    this.row = $("#" + this.id);
    this.new_row = false;

    if (this.row.length == 0) {
      this.row = $("<tr></tr>");
      this.row.attr("id", this.id);

      this.row.click(function() {
        $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
        
        let cell: any = $(this).find('td:last');
        let value = cell.text();
        Signal.emit("doSend", [["cmd", "activate"], ["par", value], ["par", "on"]]);
      });

      this.new_row = true;
      owner.append(this.row);
    }
  }

  public add_cell(id: string, text: string)
  {
    let cell = $("#" + id);
    if (cell.length == 0) {
      cell = $("<td style='height:8px'></td>");
      cell.attr("id", cell_id);
      line.append(cell);
    }
    cell.text();
  }
}
//==============================================================================
class table_t
{
  public owner: any;
  public table: any;
  public id: string;

  public cols_count: number;
  public rows_count: number;

  public rows: row_t[]; 

  constructor(id: string, cols: number, owner: any)
  {
    console.log("constructor: table_t, id: " + id);

    this.id = id;
    this.owner = owner;

    this.table = $("#" + id);
    
    this.cols_count = cols;
    this.rows_count = 0;
  }
}
//==============================================================================
class clients_table_t extends table_t
{
  constructor(id: string, cols: number, owner: any)
  {
    super(id, cols, owner);

    Signal.bind("add_client", this.add_client, this);
  }

  add_client(data: any)
  {
    if(data.PAR == undefined)
      return;

    let client: client_t = data.PAR;
    
  }

  add_row(data: any) 
  {


    

    let id: string = client[0].PAR;
    let name: string = client[1].PAR;

    let row_id: string = "client_" + id + "_" + name;
    let row: row_t = new row_t(row_id, this.table);
    if(row.new_row)
      this.rows.push(row);

    let cell_id: string = "name_" + row_id;
    row.add_cell(cell_id, name);

    cell_id = "id_" + row_id;
    row.add_cell(cell_id, id);
  }
}
//==============================================================================
class data_table_t extends table_t 
{
  constructor(id: string, cols: number, owner: any) 
  {
    super(id, cols, owner);

    Signal.bind("add_data", this.add_row, this);
  }

  add_row(data: any) {
    var line_id = data[0];
    var line = $("#" + line_id);
    if (line.length == 0) {
      line = $("<tr></tr>");
      line.attr("id", line_id);
      this.table.append(line);
    }

    var cell_id = "key_" + data[0];
    var cell = $("#" + cell_id);
    if (cell.length == 0) {
      cell = $("<td style='height:8px'></td>");
      cell.attr("id", cell_id);
      line.append(cell);
    }
    cell.text(data[1]);

    cell_id = "value_" + data[0];
    cell = $("#" + cell_id);
    if (cell.length == 0) {
      cell = $("<td style='height:8px'></td>");
      cell.attr("id", cell_id);
      line.append(cell);
    }
    cell.text(data[2]);
  }
}
//==============================================================================