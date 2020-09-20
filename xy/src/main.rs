extern crate pest;
#[macro_use]
extern crate pest_derive;

use pest::iterators::Pair;
use pest::iterators::Pairs;
use pest::Parser;
use std::env;
use std::fs::File;
use std::io::Read;

#[derive(Parser)]
#[grammar = "grammar.pest"]
struct IdentParser;

lazy_static! {
  static ref PREC_CLIMBER: PrecClimber<Rule> = {
    use Assoc::*;
    use Rule::*;

    PrecClimber::new(vec![
      Operator::new(add, Left) | Operator::new(subtract, Left),
      Operator::new(multiply, Left) | Operator::new(divide, Left),
      Operator::new(power, Right),
    ])
  };
}

fn eval(expression: Pairs<Rule>) -> f64 {
  PREC_CLIMBER.climb(
    expression,
    |pair: Pair<Rule>| match pair.as_rule() {
      Rule::num => pair
        .as_str()
        .parse::<f64>()
        .unwrap_or_else(|e| panic!("{}", e)),
      Rule::expr => eval(pair.into_inner()),
      _ => unreachable!(),
    },
    |lhs: f64, op: Pair<Rule>, rhs: f64| match op.as_rule() {
      _ => unreachable!(),
    },
  )
}

fn main() {
  let args: Vec<String> = env::args().collect();
  let mut file = File::open(&args[1]).expect("Couldn't open file");
  let mut contents = String::new();
  file
    .read_to_string(&mut contents)
    .expect("File::read didn't succeed");
  let pairs = IdentParser::parse(Rule::main, &contents).unwrap_or_else(|e| panic!("{}", e));

  for pair in pairs {
    // A pair is a combination of the rule which matched and a span of input
    println!("Rule:    {:?}", pair.as_rule());
    println!("Span:    {:?}", pair.as_span());
    println!("Text:    {}", pair.as_str());
  }
}
