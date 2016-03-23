class exec_t
{
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
      $("#last_cmd").text("Command: " + data[0][1]);

      switch (data[0][1]) 
      {
        case "clients":
        {
          for (var i = 1; i < data.length; i++) 
            Signal.emit("add_client", data[i][1].split('_'));
          break;
        }
      }
    }
    else 
    {
      for (var i = 0; i < data.length; i++) 
        Signal.emit("add_data", data[i]);
    }
  }
}