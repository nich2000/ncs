//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class map_t {
  //----------------------------------------------------------------------------
  private _id: string;
  private _canvas: any;
  private _ctx: any;
  private _is_init: boolean = false;
  private _height: number;
  private _width: number;
  private _scale: number;
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
  }
  //----------------------------------------------------------------------------
  public get_height(): number {
    return this._height;
  }
  //----------------------------------------------------------------------------
  public set_height(height: number) {
    this._height = height;
  }
  //----------------------------------------------------------------------------
  public get_width(): number {
    return this._width;
  }
  //----------------------------------------------------------------------------
  public set_width(width: number) {
    this._width = width;
  }
  //----------------------------------------------------------------------------
  public get_scale(): number {
    return this._scale;
  }
  //----------------------------------------------------------------------------
  public set_scale(height: number, width: number) {
    let tmp_scale_h: number = height / this._height;
    let tmp_scale_w: number = width / this._width;

    if (tmp_scale_h < tmp_scale_w)
      this._scale = tmp_scale_h;
    else
      this._scale = tmp_scale_w;
  }
  //----------------------------------------------------------------------------
  public clear() {
    if (!this._is_init)
      return;

    this._ctx.clearRect(0, 0, this._canvas.width, this._canvas.height);
  }
  //----------------------------------------------------------------------------
  public test_draw() {
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
