//==============================================================================
//==============================================================================
class exec_t {
  constructor() {
    console.log("constructor: exec_t");

    Signal.bind("onMessage", this.doOnMessage, this);
  }

  public doOnMessage(message: any) {
    let data: any;
    data = JSON.parse(message);

    if (data.length == 0)
      return;

    if (data[0].CMD != undefined) {
      $("#last_cmd").text("Command: " + data[0].CMD);

      switch (data[0].CMD) {
        case "clients":
          {
            for (let i = 1; i < data.length; i++)
              Signal.emit("add_client", data[i].PAR);
            break;
          }
      }
    }
    else {
      Signal.emit("add_data", data);
    }
  }
}
//==============================================================================
