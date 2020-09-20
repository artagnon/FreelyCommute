extern crate pest;
#[macro_use]
extern crate pest_derive;

use pest::Parser;
use std::env;
use std::fs::File;
use std::io::Read;

#[derive(Parser)]
#[grammar = "grammar.pest"]
struct IdentParser;

fn main() {
  let args: Vec<String> = env::args().collect();
  let mut file = File::open(&args[1]).expect("Couldn't open file");
  let mut contents = String::new();
  file
    .read_to_string(&mut contents)
    .expect("Couldn't read file");
  let pairs = IdentParser::parse(Rule::main, &contents).unwrap_or_else(|e| panic!("{}", e));

  for pair in pairs {
    // A pair is a combination of the rule which matched and a span of input
    println!("Rule:    {:?}", pair.as_rule());
    println!("Span:    {:?}", pair.as_span());
    println!("Text:    {}", pair.as_str());
  }
}
