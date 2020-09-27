extern crate pest;
#[macro_use]
extern crate pest_derive;

use pest::iterators::Pairs;
use pest::Parser;
use std::env;
use std::fs::File;
use std::io::Read;

#[derive(Parser)]
#[grammar = "grammar.pest"]
struct IdentParser;

fn recurse_pairs(pairs: Pairs<Rule>, w: usize) {
  for pair in pairs {
    print!("{:width$}", " ", width = w * 4);
    println!("{:?} {}", pair.as_rule(), pair.as_str());
    recurse_pairs(pair.into_inner(), w + 1);
  }
}

fn main() {
  let args: Vec<String> = env::args().collect();
  let mut file = File::open(&args[1]).expect("Couldn't open file");
  let mut contents = String::new();
  file
    .read_to_string(&mut contents)
    .expect("Couldn't read file");
  let pairs = IdentParser::parse(Rule::main, &contents).unwrap_or_else(|e| panic!("{}", e));
  recurse_pairs(pairs, 0);
}
