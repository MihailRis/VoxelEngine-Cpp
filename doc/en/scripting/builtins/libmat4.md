# *mat4* library

*mat4* contains a set of functions for work with transformation 4x4 matrices.

Most functions have several options for argument lists (overloads).

## Data types

Type conventions will be used on this page.
- vector - an array of three or four numbers
- vec3 - array of three numbers
- matrix - array of 16 numbers - matrix

>[!ATTENTION]
> Type annotations are part of the documentation and are not present in Lua.

## Identity matrix - *mat4.idt(...)*

```lua
-- creates an identity matrix
mat4.idt()

-- writes an identity matrix to dst
mat4.idt(dst: matrix)
```

## Matrix multiplication - *mat4.mul(...)*

```lua
-- returns the result of matrix multiplication
mat4.mul(a: matrix, b: matrix)
-- writes the result of matrix multiplication to dst
mat4.mul(a: matrix, b: matrix, dst: matrix)

-- returns the result of multiplying a matrix and a vector
mat4.mul(a: matrix, v: vector)
-- writes the result of matrix and vector multiplication to dst
mat4.mul(a: matrix, v: vector, dst: vector)
```

## Matrix inversion - *mat4.inverse(...)*

```lua
-- returns the result of matrix inversion
mat4.inverse(m: matrix)
-- writes the result of matrix inversion to dst
mat4.inverse(m: matrix, dst: matrix)
```

## Matrix transposition - *mat4.transpose(...)*

```lua
-- returns the result of matrix transposition
mat4.transpose(m: matrix)
-- writes the result of matrix transposition to dst
mat4.transpose(m: matrix, dst: matrix)
```

## Offset - *mat4.translate(...)*

```lua
-- creates a translation matrix
mat4.translate(translation: vec3)
-- returns the result of applying a translation to matrix m
mat4.translate(m: matrix, translation: vec3)
-- writes the result of applying a translation to matrix m to dst
mat4.translate(m: matrix, translation: vec3, dst: matrix)
```
## Scaling - *mat4.scale(...)*

```lua
-- creates a scaling matrix
mat4.scale(scale: vec3)
-- returns the result of applying scaling to matrix m
mat4.scale(m: matrix, scale: vec3)
-- writes the result of applying scaling to matrix m to dst
mat4.scale(m: matrix, scale: vec3, dst: matrix)
```

## Rotation - *mat4.rotate(...)*

The angle of rotation is indicated in degrees.

```lua
-- creates a rotation matrix (angle - rotation angle) along a given axis (axis is an unit vector)
mat4.rotate(axis: vec3, angle: number)
-- returns the result of applying rotation to matrix m
mat4.rotate(m: matrix, axis: vec3, angle: number)
-- writes the result of applying rotation to matrix m to dst
mat4.rotate(m: matrix, axis: vec3, angle: number, dst: matrix)
```

## Translation to string - *mat4.tostring(...)*

```lua
-- returns a string representing the contents of the matrix
mat4.tostring(m: matrix)
-- returns a string representing the contents of the matrix, human-readable if multiline = true
mat4.tostring(m: matrix, multiline: bool)
```
