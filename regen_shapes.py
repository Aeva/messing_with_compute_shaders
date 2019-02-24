#!/usr/bin/env python3

from math import *
from ctypes import c_uint, c_float, sizeof

c_vec4 = c_float*4
c_mat4 = c_float*16

SPHERE_SIZE = sizeof(c_vec4)
BOX_SIZE = sizeof(c_vec4) + sizeof(c_mat4) * 2

SPHERES = (
    (0, 0, 0, 200),
    (-50, -50, 100, -150),
    (100, 100, 100, -80),
    (-10, -10, -100, -100)
)

BOXES = (
    ((100, 0, 55), (0, 45, 0), (60, 400, 60), -1),
    ((-100, -100, 0), (0, 0, 45), (50, 50, 400), -1)
)

IDENTITY = (1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1)


def make_translation(x, y, z):
    matrix = list(IDENTITY)
    matrix[12] = x
    matrix[13] = y
    matrix[14] = z
    return tuple(matrix)


def fill_rotation(indices, values):
    matrix = list(IDENTITY)
    for i, v in zip(indices, values):
        matrix[i] = v
    return tuple(matrix)


def make_rotate_x(rotation):
    rsin = sin(radians(rotation))
    rcos = cos(radians(rotation))
    return fill_rotation(
        (5, 6, 9, 10),
        (rcos, -rsin, rsin, rcos))


def make_rotate_y(rotation):
    rsin = sin(radians(rotation))
    rcos = cos(radians(rotation))
    return fill_rotation(
        (0, 2, 8, 10),
        (rcos, rsin, -rsin, rcos))


def make_rotate_z(rotation):
    rsin = sin(radians(rotation))
    rcos = cos(radians(rotation))
    return fill_rotation(
        (0, 1, 4, 5),
        (rcos, rsin, -rsin, rcos))


def matxmat(lhs, rhs):
    matrix = list(range(16))
    for y in range(4):
        for x in range(4):
            matrix[x * 4 + y] = \
                lhs[y +  0] * rhs[x * 4 + 0] + \
                lhs[y +  4] * rhs[x * 4 + 1] + \
                lhs[y +  8] * rhs[x * 4 + 2] + \
                lhs[y + 12] * rhs[x * 4 + 3]
    return tuple(matrix)


if __name__ == "__main__":
    with open("shape.blob", "wb") as blob:
        blob.write(c_uint(len(SPHERES)))
        blob.write(c_uint(len(BOXES)))
        blob.write(c_uint(SPHERE_SIZE))
        blob.write(c_uint(BOX_SIZE))

        for Params in SPHERES:
            blob.write(c_vec4(*Params))

        for position, rotate, extent, mode in BOXES:
            blob.write(c_vec4(*extent, mode))
            translation = make_translation(*position)
            rotation = IDENTITY
            if rotate[0] != 0:
                rotation = make_rotate_x(rotate[0])
            elif rotate[1] != 0:
                rotation = make_rotate_y(rotate[1])
            elif rotate[2] != 0:
                rotation = make_rotate_z(rotate[2])
            blob.write(c_mat4(*matxmat(translation, rotation)))
            blob.write(c_mat4(*rotation))

