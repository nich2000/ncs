//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class map_item_t {
  //----------------------------------------------------------------------------
  private _kind:   string;
  private _number: number;
  private _index:  number;
  private _lat_f:  number;
  private _lon_f:  number;
  private _lat:    number;
  private _lon:    number;
  //----------------------------------------------------------------------------
  constructor(line: any) {
    console.log(line);

    this._kind   = line[0].KND;
    this._number = line[1].NUM;
    this._index  = line[2].IND;
    this._lat_f  = line[3].LAF;
    this._lon_f  = line[4].LOF;
    this._lat    = line[5]._LA;
    this._lon    = line[6]._LO;
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class map_t {
  //----------------------------------------------------------------------------
  private _id:      string  = "";
  private _canvas:  any     = undefined;
  private _ctx:     any     = undefined;
  private _is_init: boolean = false;
  private _height:  number  = 1;
  private _width:   number  = 1;
  private _scale:   number  = 1;
  private _items:   Array<map_item_t> = [];
  //----------------------------------------------------------------------------
  constructor(id: string) {
    console.log("constructor: map_t, id: " + id);

    this._id = id;
    this._canvas = $("#" + id)[0];

    let canvasSupported: boolean = !!document.createElement("canvas").getContext;

    if (canvasSupported) {
      this._ctx = this._canvas.getContext('2d');

      this._height = this._canvas.height;
      this._width = this._canvas.width;

      this._is_init = true;
    }
    else {
      console.error('Can not get context');
      this._is_init = false;
    }

    this.clear();

    Signal.bind("map",  this.load_map, this);
  }
  //----------------------------------------------------------------------------
  private set_scale(height: number, width: number) {
    let tmp_scale_h: number = height / this._height;
    let tmp_scale_w: number = width / this._width;

    if (tmp_scale_h < tmp_scale_w)
      this._scale = tmp_scale_h;
    else
      this._scale = tmp_scale_w;
  }
  //----------------------------------------------------------------------------
  private clear() {
    if (!this._is_init)
      return;

    this._ctx.clearRect(0, 0, this._canvas.width, this._canvas.height);
  }
  //----------------------------------------------------------------------------
  private load_map(data: any): void {
    for(let i: number = 0; i < data.length; i++){
      let map_item: map_item_t = new map_item_t(data[i].PAR);
    }
  }
  //----------------------------------------------------------------------------
  private draw_map(): void {
  }
  //----------------------------------------------------------------------------
  public test_draw(): void {
    if (!this._is_init)
      return;

    this.clear();

    this._ctx.beginPath();
    this._ctx.moveTo(170, 80);
    this._ctx.bezierCurveTo(130, 100, 130, 150, 230, 150);
    this._ctx.bezierCurveTo(250, 180, 320, 180, 340, 150);
    this._ctx.bezierCurveTo(420, 150, 420, 120, 390, 100);
    this._ctx.bezierCurveTo(430, 40, 370, 30, 340, 50);
    this._ctx.bezierCurveTo(320, 5, 250, 20, 250, 50);
    this._ctx.bezierCurveTo(200, 5, 150, 20, 170, 80);
    this._ctx.closePath();

    this._ctx.lineWidth = 5;
    this._ctx.strokeStyle = 'blue';
    this._ctx.stroke();
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
