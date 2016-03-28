function asInt(value: any) {
    var result = [
        (value & 0x000000ff),
        (value & 0x0000ff00) >> 8,
        (value & 0x00ff0000) >> 16,
        (value & 0xff000000) >> 24
    ];

    return result;
}

function toInt(value: any) {
    var result = 0;

    for (var i = 0; i < 4; i++) // 4 bytes for integer
    {
        result += value[i] << (i * 8);
    }

    return result;
}

function asDouble(value: any) {
  var result: any[];
  var binary = new Float64Array([value]);
  var array = new Uint8Array(binary.buffer);

  for (var i = 0; i < array.length; i++) {
    result.push(array[i]);
  }

  return result;
}

function toDouble(value: any) {
  var array = new Uint8Array(value);
  return new Float64Array(array.buffer)[0];
}

function toNumbersArray(value: any) {
  var buffer: any[];

  for (var i = 0; i < value.length; i++) {
    buffer[i] = value[i];
  }

  return buffer;
}

function toNumberToByte(value: any) {
  var arr = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 'A', 'B', 'C', 'D', 'E', 'F'];
  return arr[value >> 4] + '' + arr[value & 0xF];
}