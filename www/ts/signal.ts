/*
class Socket
{
  public onmessage(data: any): void
  {
    Signal.emit('onmessage', data);
  }
}
class OtherClass
{
  public constructor()
  {
    Signal.bind('onmessage', proceedData);
  }
  public proceedData(data: any)
  {
    console.log(data);
  }
}
*/
//==============================================================================
interface ISignal {
    signal: string;
    method: Function;
    context?: any;
}
//==============================================================================
class Signal {
  private static signals: Array<ISignal> = [];

  public static bind(signal: string, method: Function, context?: any): void {
    let tmp: ISignal =
      {
        signal: signal,
        method: method
      }

    if (context)
      tmp.context = context;

    this.signals.push(tmp);
  }

  public static emit(signal: string, data: any): void {
    for (let key in this.signals) {
      if (this.signals[key].signal == signal) {
        if (this.signals[key].context) {
          this.signals[key].method.call(this.signals[key].context, data);
        }
        else {
          this.signals[key].method(data);
        }
      }
    }
  }
}
//==============================================================================
