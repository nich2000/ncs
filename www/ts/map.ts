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
  private _cnv:        any     = "";
  private _canvas:     any     = undefined;
  private _ctx:        any     = undefined;
  private _is_init:    boolean = false;
  private _height:     number  = 1;
  private _width:      number  = 1;
  private _scale:      number  = 1;

  private _min_h:      number  =  1000000000;
  private _max_h:      number  = -1000000000;
  private _min_w:      number  =  1000000000;
  private _max_w:      number  = -1000000000;
  private _height_map: number  = 1;
  private _width_map:  number  = 1;

  private _map_items:       Array<map_item_t> = [];
  private _position_first:  Array<map_item_t> = [];
  private _position_second: Array<map_item_t> = [];
  //----------------------------------------------------------------------------
  constructor(id: string) {
    console.log("constructor: map_t, id: " + id);

    this._id = id;
    this._cnv = $("#" + id);
    this._canvas = this._cnv[0];

    let canvasSupported: boolean = !!document.createElement("canvas").getContext;
    if (canvasSupported) {
      this._ctx = this._canvas.getContext('2d');

      this._height = this._canvas.height;
      this._width = this._canvas.width;

      this.clear();

      this._is_init = true;
    }
    else {
      console.error('Can not get context');
      this._is_init = false;
      return;
    }

    // TODO: http://jquery-howto.blogspot.ru/2010/02/dynamically-create-iframe-with-jquery.html
    this._cnv.click(function() {
      // let modal = element.add("modal_map", "<iframe/>", $("body"));
      // element.set_src("modal_map", "modal_map.html");
      // modal.onclick = function () {
        // this.parentElement.removeChild(this);
      // };
    });

    Signal.bind("map", this.load_map, this);
    Signal.bind("add_position", this.add_position, this);
  }
  //----------------------------------------------------------------------------
  private load_map(data: any): void {
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

      this._map_items.push(map_item);
    }

    // bounds
    this.set_bounds();

    // scale
    this.set_scale();

    // draw
    this.refresh();
  }
  //----------------------------------------------------------------------------
  private set_bounds(): void {
    for(let i = 0; i < this._map_items.length; i++){
      let lat: number = this._map_items[i].lat;
      let lon: number = this._map_items[i].lon;

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
    let tmp_scale_h: number = this._height / this._height_map;
    let tmp_scale_w: number = this._width  / this._width_map;

    if (tmp_scale_h < tmp_scale_w)
      this._scale = tmp_scale_h;
    else
      this._scale = tmp_scale_w;
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
  // TOOD: https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API/Tutorial
  // http://shpargalkablog.ru/2011/02/modalnoe-okno-css.html
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
      let lat = (this._map_items[i].lat - this._min_h) * this._scale;
      let lon = (this._map_items[i].lon - this._min_w) * this._scale;
      this._ctx.arc(lon, lat, 1, 0, 2 * Math.PI);
    }
  }
  //----------------------------------------------------------------------------
  private draw_client(): void {
    for(let i = 0; i < this._position_first.length; i++){
      let lat = (this._position_first[i].lat - this._min_h) * this._scale;
      let lon = (this._position_first[i].lon - this._min_w) * this._scale;
      this._ctx.arc(lon, lat, 1, 0, 2 * Math.PI);
    }
  }
  //----------------------------------------------------------------------------
  public refresh(): void {
    if (!this._is_init)
      return;

    this.begin_draw();

    this.draw_map();
    this.draw_client();
    // this.draw_client();

    this.end_draw();
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
