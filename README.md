## Hack Emulator

This program takes Hack machine code as input and executes it.

### TODO

- [ ] Variables should be `> 1024` not `< 1024`
- [ ] Bug: interpreting zero-byte (presumably EOF) as A-instruction

### Instruction Set

Hack machine code has two types of instructions: 'A' and 'C'.

- A-type instructions simply setup C-type instructions by manipulating the
  A-register.
- C-type instructions perform some computation operation.

### A-type

```
0 000000000100101
```

- Leading `0` means A-type.
- Remaining `15` bits are a value to store into the A-register.
- Written in assembly as `@37` (load decimal 37 to A-register).

### C-type

```
1  _ _ a c1 c2 c3 c4 c5 c6 d1 d2 d3 j1 j2 j3
```

- Leading `1` means C-type.
- Proceeding 2 bits are ignored.
- `a`-bit and the 6 `c`-bits represent the computation to be executed.

![table of instructions](https://i.imgur.com/8U5JMeG.png)

Destination bits (`d1`, `d2` & `d3`)

![destination table](https://i.imgur.com/VBOmxBu.png)

Jump bits (`j1`, `j2`, `j3`)

![jump table](https://i.imgur.com/XWf6hSo.png)
