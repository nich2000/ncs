//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class cell_t
{
  public owner: any;
  public table: any;
  public id: string;

  public index: number;

  public text: string;
  public html: string;

  constructor(id: string, text: string)
  {
    this.id = id;
    this.text = text;
  }
}
//==============================================================================
class row_t
{
  public owner: any;
  public table: any;
  public id: string;

  public index: number;
  public count: number;

  public cells: cell_t[];
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

  constructor(id: string, cols: number)
  {
    console.log("constructor: table_t, id: " + id);

    this.owner = $(window);
    this.table = $("#" + id);
    this.id = id;
    this.cols_count = cols;
    this.rows_count = 0;
  }
}
//==============================================================================
class clients_table_t extends table_t
{
  constructor(id: string, cols: number)
  {
    super(id, cols);

    Signal.bind("add_client", this.add_row, this);
  }

  add_row(client: any) 
  {
    var line_id = "client_" + client[0] + "_" + client[1];
    var line = $("#" + line_id);
    if (line.length == 0) {
      line = $("<tr></tr>");
      line.attr("id", line_id);
      line.click(function() {
        $(this).addClass('dems-selected').siblings().removeClass('dems-selected');
        var cell: any;
        cell = $(this).find('td:last');
        var value = cell.text();
        Signal.emit("doSend", value);
      });
      this.table.append(line);
      this.rows_count++;
    }

    var cell_id = "name_" + line_id;
    var cell = $("#" + cell_id);
    if (cell.length == 0) {
      cell = $("<td style='height:8px'></td>");
      cell.attr("id", cell_id);
      line.append(cell);
    }
    cell.text(client[0]);

    var cell_id = "id_" + line_id;
    var cell = $("#" + cell_id);
    if (cell.length == 0) {
      cell = $("<td style='height:8px'></td>");
      cell.attr("id", cell_id);
      line.append(cell);
    }
    cell.text(client[1]);
  }
}
//==============================================================================
class data_table_t extends table_t 
{
  constructor(id: string, cols: number) 
  {
    super(id, cols);

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
    cell.text(data[0]);

    cell_id = "value_" + data[0];
    cell = $("#" + cell_id);
    if (cell.length == 0) {
      cell = $("<td style='height:8px'></td>");
      cell.attr("id", cell_id);
      line.append(cell);
    }
    cell.text(data[1]);
  }
}
//==============================================================================