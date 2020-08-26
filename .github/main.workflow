workflow "Travis CI" {
  on:
  schedule:
    - cron: "0 0 * * *"
  resolves = ["Travis CI"]
}

action "Travis CI" {
  uses = "BanzaiMan/travis-ci-action@master"
  secrets = ["TRAVIS_TOKEN"]
  env = {
    SLUG = "qurious-pixel/yuzu"
    SITE = "com"
  }
} # Filter for a new tag
