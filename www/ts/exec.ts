class exec_t
{
  private static_filter: string[] =
  [
  "_ID",                 // 1
  "TIM",                 // 2
  // "T_S",                 // 3
  // "CNT",                 // 4
  "SPD",                 // 5
  "HEA",                 // 6
  "LAT",                 // 7
  "LON"                  // 8
  // "UN1",                 // 9
  // "UN2",                 // 10
  // "AZ1",                 // 11
  // "AZ2",                 // 12
  // "MT1",                 // 13
  // "MT2",                 // 14
  // "BVL",                 // 15
  // "UN3",                 // 16
  // "UN4",                 // 17
  // "EVL",                 // 18
  // "USB"                  // 19
 ];
  
  constructor()
  {
    console.log("constructor: exec_t");
    
    Signal.bind("onMessage", this.doOnMessage, this);
  }

  public doOnMessage(message: any)
  {
    var data: string;
    data = JSON.parse(message);

    if (data.length == 0)
      return;
    if (data[0].length == 0)
      return;

    if (data[0][0] == "CMD") 
    {
      $("#last_cmd").text("Command: " + data[0][2]);

      switch (data[0][2]) 
      {
        case "clients":
        {
          for (var i = 1; i < data.length; i++) 
            Signal.emit("add_client", data[i][2].split('_'));
          break;
        }
      }
    }
    else 
    {
      for (var i = 0; i < data.length; i++) 
      {
        if(this.static_filter.indexOf(data[i][0]) != -1)
          Signal.emit("add_data", data[i]);
      }
    }
  }
}
