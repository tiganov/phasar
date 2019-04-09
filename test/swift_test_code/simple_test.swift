func sink(value:Int) -> () {
  // sink method
}

func source() -> Int {
  return -1;
}

var a:Int = source();
sink(value:a);
