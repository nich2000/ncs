//==============================================================================
/// <reference path="./jquery.d.ts"/>
//==============================================================================
class element {
  public static add(id: string, tag: string, owner: any): any{
    let tmp = $(tag);
    tmp.attr("id", id);
    owner.append(tmp);
    return tmp;
  }

  public static delete(): void{
  }

  public static find_by_id(id: string): any{
    return $("#" + id)[0];
  }

  public static exists_by_id(id: string): boolean {
    return $("#" + id).length > 0;
  }
}
//==============================================================================
