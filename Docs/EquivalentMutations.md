# Equivalent Mutations

Equivalent mutations is a hot topic around mutation testing. Here we collect
equivalent mutations we found during our analysis of various projects.

### Remove Void Call removes assertions

You remove `assert(...)` call and nothing happens. This is not done yet but we
can teach Mull to ignore this mutation.

### Performance branches

```c
if (condition) {
  fast code
} else {
  slow code
}
```

To kill this equivalent mutation you can write a test that asserts that indeed
a fast code is run if you provide a given condition and vice versa.

### std::vector::reserve

`std::vector::reserve(number)`

We can mutate the number and the tests will still pass. We can write a
filter for this function to exclude the mutations from being generated.

### Remove Void Call and Replace Call remove free() or close() calls and nothing happens

We are not sure about the best strategy here but this case seems to be also
approachable.

### Conditionals Boundary

```c
if (level === currLevel) {
  // ...
}
else if (level > currLevel) { // <-- Conditionals Boundary does >= and nothing changes.
  // ...
}
else {
  // ...
}
```
