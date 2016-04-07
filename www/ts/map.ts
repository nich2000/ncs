//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class map_item_t {
  //----------------------------------------------------------------------------
  // public kind:   string = "";
  // public number: number = 0;
  // public index:  number = 0;
  public lat_f:  number = 0;
  public lon_f:  number = 0;
  // public lat:    number = 0;
  // public lon:    number = 0;
  //----------------------------------------------------------------------------
  // constructor(line: any) {
  //   console.log(line);

  //   this.kind   = line[0].KND;
  //   this.number = line[1].NUM;
  //   this.index  = line[2].IND;
  //   this.lat_f  = line[3].LAF;
  //   this.lon_f  = line[4].LOF;
  //   this.lat    = line[5]._LA;
  //   this.lon    = line[6]._LO;
  // }
  //----------------------------------------------------------------------------
  constructor(lat: number, lon: number) {
    this.lat_f  = lat;
    this.lon_f  = lon;
  }
  //----------------------------------------------------------------------------
  public get lat() : number {
    return this.lat_f;
  }
  //----------------------------------------------------------------------------
  public get lon() : number {
    return this.lon_f;
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
class map_t {
  //----------------------------------------------------------------------------
  private _id:         string  = "";
  private _canvas:     any     = undefined;
  private _ctx:        any     = undefined;
  private _is_init:    boolean = false;
  private _height:     number  = 1;
  private _width:      number  = 1;
  private _height_map: number  = 1;
  private _width_map:  number  = 1;
  private _scale:      number  = 1;

  private _map_items:      Array<map_item_t>  = [];
  private _position_first: Array<map_item_t>  = [];
  private _position_second: Array<map_item_t> = [];
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

    Signal.bind("map", this.load_map, this);
    Signal.bind("add_position", this.add_position, this);
  }
  //----------------------------------------------------------------------------
  private load_map(data: any): void {
    this._map_items = [];
    for(let i: number = 0; i < data.length; i++){
      // let map_item: map_item_t = new map_item_t(data[i].PAR);
      let map_item: map_item_t = new map_item_t(data[i].PAR[3].LAF, data[i].PAR[4].LOF);
      this._map_items.push(map_item);
    }

    // min max size - width, height of real map
    let min_h: number =  1000000000;
    let max_h: number = -1000000000;
    let min_w: number =  1000000000;
    let max_w: number = -1000000000;

    for(let i = 0; i < this._map_items.length; i++){
      let lat: number = this._map_items[i].lat;
      let lon: number = this._map_items[i].lon;

      if (lat > max_h)
        max_h = lat;
      if(lat < min_h)
        min_h = lat;
      if(lon > max_w)
        max_w = lon;
      if(lon < min_w)
        min_w = lon;
    }

    this._height_map = max_h - min_h;
    this._width_map  = max_w - min_w;

    // scale
    this.set_scale();

    // draw
    this.refresh();
  }
  //----------------------------------------------------------------------------
  private add_position(lat: number, lon: number, active: active_t): void {
    let map_item: map_item_t = new map_item_t(lat, lon);

    if(active == active_t.first)
      this._position_first.push(map_item);
    else if(active = active_t.second)
      this._position_second.push(map_item);

    this.refresh();
  }
  //----------------------------------------------------------------------------
  private set_scale(): void {
    let tmp_scale_h: number = this._height_map / this._height;
    let tmp_scale_w: number = this._width_map / this._width;

    if (tmp_scale_h < tmp_scale_w)
      this._scale = tmp_scale_h;
    else
      this._scale = tmp_scale_w;
  }
  //----------------------------------------------------------------------------
  // TOOD: https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API/Tutorial
  //----------------------------------------------------------------------------
  private clear(): void {
    if (!this._is_init)
      return;

    this._ctx.clearRect(0, 0, this._canvas.width, this._canvas.height);
  }
  //----------------------------------------------------------------------------
  private begin_draw(): void {
    this.clear();
  }
  //----------------------------------------------------------------------------
  private end_draw(): void {
    this._ctx.stroke();
  }
  //----------------------------------------------------------------------------
  private draw_map(): void {
    for(let i = 0; i < this._map_items.length; i++){
      let lat = this._map_items[i].lat * this._scale;
      let lon = this._map_items[i].lon * this._scale;
      this._ctx.arc(lat, lon, 2, 0, 2 * Math.PI);
    }
  }
  //----------------------------------------------------------------------------
  private draw_client(): void {
    for(let i = 0; i < this._position_first.length; i++){
      let lat = this._position_first[i].lat * this._scale;
      let lon = this._position_first[i].lon * this._scale;
      this._ctx.arc(lat, lon, 1, 0, 2 * Math.PI);
    }
  }
  //----------------------------------------------------------------------------
  public refresh(): void {
    this.begin_draw();

    this.draw_map();
    this.draw_client();
    // this.draw_client();

    this.end_draw();
  }
  //----------------------------------------------------------------------------
  public test_draw(): void {
    if (!this._is_init)
      return;

    this.begin_draw();

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

    this.end_draw();
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
