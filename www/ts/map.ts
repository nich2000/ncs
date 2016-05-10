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
  private _test_mode:  boolean = false;
  //----------------------------------------------------------------------------
  private _id:         string  = "";
  private _cnv:        any     = undefined; // jquery object
  private _canvas:     any     = undefined; // DOM object
  private _ctx:        any     = undefined;
  private _is_init:    boolean = false;
  private _maximaize:  boolean = false;
  //----------------------------------------------------------------------------
  private _height:     number  = 1;
  private _width:      number  = 1;
  private _scale:      number  = 1;
  //----------------------------------------------------------------------------
  private _min_h:      number  =  1000000000;
  private _max_h:      number  = -1000000000;
  private _min_w:      number  =  1000000000;
  private _max_w:      number  = -1000000000;
  private _height_map: number  = 1;
  private _width_map:  number  = 1;
  //----------------------------------------------------------------------------
  private _map_items:       Array<map_item_t> = [];
  private _position_first:  Array<map_item_t> = [];
  private _position_second: Array<map_item_t> = [];
  //----------------------------------------------------------------------------
  constructor(id: string) {
    console.log("constructor, map_t, id: " + id);

    this._id = id;
    this._cnv = $("#" + id);
    this._canvas = this._cnv[0];

    let canvasSupported: boolean = !!document.createElement("canvas").getContext;
    if (canvasSupported) {
      this._ctx = this._canvas.getContext("2d");

      // this._height = this._canvas.height;
      // this._width  = this._canvas.width;

      this._height = this._cnv.height();
      this._width  = this._cnv.width();

      this._ctx.canvas.height = this._height;
      this._ctx.canvas.width = this._width;

      this.clear();

      this._is_init = true;
    }
    else {
      console.error("Can not get context");
      this._is_init = false;
      return;
    }

    this._cnv.click(function() {
      this._maximaize = !this._maximaize;

      if(this._maximaize){
        $("#maxi_map").append($("#canvas_map"));
        $("#maxi_map").removeClass("dems-hide");
        $("#maxi_map").addClass("dems-show");
      }
      else{
        $("#mini_map").append($("#canvas_map"));
        $("#maxi_map").removeClass("dems-show");
        $("#maxi_map").addClass("dems-hide");
      }

      this.refresh();
    });

    Signal.bind("map", this.load_map, this);
    Signal.bind("add_position", this.add_position, this);
  }
  //----------------------------------------------------------------------------
  public get test_mode() : boolean {
    return this._test_mode;
  }
  //----------------------------------------------------------------------------
  public set test_mode(v : boolean) {
    this._test_mode = v;
    this.refresh();
  }
  //----------------------------------------------------------------------------
  private debug_draw(): void {
    console.log("map, debug_draw");

    this._ctx.font = "15px Courier";
    this._ctx.fillStyle = 'blue';
    this._ctx.fillText(this._canvas.height + " * " + this._canvas.width, 10, 20);
    this._ctx.fillText(this._cnv.height()  + " * " + this._cnv.width(),  10, 40);
    this._ctx.fillText(this._height        + " * " + this._width,        10, 60);
    this._ctx.fillText(this._scale,                                      10, 80);
    this._ctx.fillText(this._min_h         + " * " + this._min_w,        10, 100);
    this._ctx.fillText(this._max_h         + " * " + this._max_w,        10, 120);
    this._ctx.fillText(this._height_map    + " * " + this._width_map,    10, 140);
  }
  //----------------------------------------------------------------------------
  private test_draw(): void {
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
    this._ctx.strokeStyle = "blue";
  }
  //----------------------------------------------------------------------------
  private load_map(data: any): void {
    console.log("map, load_map, size: " + data.length);

    this._map_items = [];
    for(let i: number = 0; i < data.length; i++){
      // let map_item: map_item_t = new map_item_t(data[i].PAR);

      let lon_s: string = data[i].PAR[3].LAF;
      lon_s = lon_s.replace(/\,/, ".");
      let lon: number = parseFloat(lon_s);

      let lat_s: string = data[i].PAR[4].LOF;
      lat_s = lat_s.replace(/\,/, ".");
      let lat: number = parseFloat(lat_s);

      let map_item: map_item_t = new map_item_t(lat, lon);

      // console.log(lat + "  " + lon);

      this._map_items.push(map_item);
    }

    // bounds
    this.set_bounds();

    // scale
    this.set_scale();

    this.debug_draw();

    this.draw_map();

    // draw
    // this.refresh();
  }
  //----------------------------------------------------------------------------
  private set_bounds(): void {
    console.log("map, set_bounds");

    for(let i = 0; i < this._map_items.length; i++){
      let lat: number = this._map_items[i].lat;
      let lon: number = this._map_items[i].lon;

      if((lat == 0) && (lon == 0))
        continue;

      if (lat > this._max_h)
        this._max_h = lat;

      if(lat < this._min_h)
        this._min_h = lat;

      if(lon > this._max_w)
        this._max_w = lon;

      if(lon < this._min_w)
        this._min_w = lon;
    }

    this._height_map = this._max_h - this._min_h;
    this._width_map  = this._max_w - this._min_w;
  }
  //----------------------------------------------------------------------------
  private set_scale(): void {
    console.log("map, set_scale");

    let tmp_scale_h: number = this._height / this._height_map;
    let tmp_scale_w: number = this._width  / this._width_map;

    if (tmp_scale_h < tmp_scale_w)
      this._scale = tmp_scale_h;
    else
      this._scale = tmp_scale_w;
  }
  //----------------------------------------------------------------------------
  private add_position(data: any): void {
    let lon_s: string = data[6].LAT;
    lon_s = lon_s.replace(/\,/, ".");
    let lon: number = parseFloat(lon_s);

    let lat_s: string = data[7].LON;
    lat_s = lat_s.replace(/\,/, ".");
    let lat: number = parseFloat(lat_s);

    // console.log(lat + "  " + lon);

    this._position_first = [];
    let map_item: map_item_t = new map_item_t(lat, lon);
    this._position_first.push(map_item);

    // if(active == active_t.first) {
    //   console.log("add_position, first");
    //   this._position_first.push(map_item);
    // }
    // else if(active = active_t.second) {
    //   console.log("add_position, second");
    //   this._position_second.push(map_item);
    // }

    this.refresh();
  }
  //----------------------------------------------------------------------------
  // TOOD: https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API/Tutorial
  // http://shpargalkablog.ru/2011/02/modalnoe-okno-css.html
  //----------------------------------------------------------------------------
  private clear(): void {
    if (!this._is_init)
      return;

    // console.log("map, clear");

    // this._canvas.width = this._canvas.width;

    // this._ctx.clearRect(0, 0, this._canvas.width, this._canvas.height);
    this._ctx.clearRect(0, 0, this._cnv.width(), this._cnv.height());
  }
  //----------------------------------------------------------------------------
  private begin_draw(): void {
    // console.log("map, begin_draw");

    this.clear();
  }
  //----------------------------------------------------------------------------
  private end_draw(): void {
    // console.log("map, end_draw");

    // this._ctx.stroke();
  }
  //----------------------------------------------------------------------------
  private draw_map(): void {
    console.log("map, draw_map, count: " + this._map_items.length);

    this._ctx.beginPath;
    this._ctx.strokeStyle="Blue";

    for(let i = 0; i < this._map_items.length; i++){
      let lat = (this._map_items[i].lat - this._min_h) * this._scale;
      let lon = (this._map_items[i].lon - this._min_w) * this._scale;

      this._ctx.moveTo(lon, lat);
      this._ctx.arc(lon, lat, 1, 0, 2 * Math.PI);
    }

    this._ctx.closePath();
    this._ctx.stroke();
  }
  //----------------------------------------------------------------------------
  private draw_client(): void {
    console.log("map, draw_client, count: " + this._position_first.length);

    this._ctx.beginPath;
    this._ctx.strokeStyle="Orange";

    for(let i = 0; i < this._position_first.length; i++){
      let lat = (this._position_first[i].lat - this._min_h) * this._scale;
      let lon = (this._position_first[i].lon - this._min_w) * this._scale;

      this._ctx.moveTo(lon, lat);
      this._ctx.arc(lon, lat, 3, 0, 2 * Math.PI);
    }

    this._ctx.closePath();
    this._ctx.stroke();
  }
  //----------------------------------------------------------------------------
  public refresh(): void {
    if (!this._is_init)
      return;

    // console.log("map, refresh");

    // this.begin_draw();

    // this.debug_draw();

    // if(this._test_mode)
      // this.test_draw()
    // else{
      // this.draw_map();
      this.draw_client();
      // this.draw_client();
    // }

    // this.end_draw();
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
