//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
enum pos_t {
  append = 0,
  prepend
}
//==============================================================================
class element {
  //----------------------------------------------------------------------------
  public static add(id: string, tag: string, owner: any, pos: pos_t = pos_t.append): any{
    let tmp: any = this.find_by_id(id);
    if(tmp == undefined)
    {
      tmp = $(tag);
      tmp.attr("id", id);

      if(pos == pos_t.append)
        owner.append(tmp);
      else
        owner.prepend(tmp);
    }
    return tmp;
  }
  //----------------------------------------------------------------------------
  public static delete(): void{
  }
  //----------------------------------------------------------------------------
  public static find_by_id(id: string): any{
    let res: any = $("#" + id);
    if(res.length > 0)
      return res;
    else
      return undefined;
  }
  //----------------------------------------------------------------------------
  public static exists_by_id(id: string): boolean {
    let res: any = $("#" + id);
    return res.length > 0;
  }
  //----------------------------------------------------------------------------
  public static get_text(id: string): string {
    let res: any = $("#" + id);
    return res.text();
  }
  //----------------------------------------------------------------------------
  public static set_text(id: string, text: string): void {
    let res: any = $("#" + id);
    res.text(text);
  }
  //----------------------------------------------------------------------------
  public static set_src(id: string, src: string): void {
    let res: any = $("#" + id);
    res.attr("src", src);
  }
  //----------------------------------------------------------------------------
}
//==============================================================================
