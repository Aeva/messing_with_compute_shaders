#!/usr/bin/env racket
#lang racket

(require "glue/math_stuff.scm")

;; Some size constants that will be needed for:
(define vec4-size (* 4 4))
(define mat4-size (* vec4-size 4))
(define sphere-size vec4-size)
(define box-size (+ vec4-size (* mat4-size 2)))

;; Methods for writing numbers to disk so C++ can read them:
(define (write-uint32 x) (write-bytes (integer->integer-bytes x 4 #f)))
(define (write-uint32* x) (if (number? x) (write-uint32 x) (map write-uint32 x)))
(define (write-float x) (write-bytes (real->floating-point-bytes x 4)))
(define (write-float* x) (if (number? x) (write-float x) (map write-float x)))


;; Shapes to be rendered:
(define spheres '((0 0 0 200)
		  (-50 -50 100 -150)
		  (100 100 100 -80)
		  (-10 -10 -100 -100)))
(define boxes '(((100 0 55) (0 45 0) (60 400 60) -1)
		((-100 -100 0) (0 0 45) (50 50 400) -1)))


;; Read shape data and dump it to disk:
(define (write-blob)
  (write-uint32*
   (list (length spheres)
	 (length boxes)
	 sphere-size
	 box-size))
  (map write-float* spheres)
  (map (lambda (box)
	 (let* ([translation (apply translate (car box))]
		[rot-x (list-ref (cadr box) 0)]
		[rot-y (list-ref (cadr box) 1)]
		[rot-z (list-ref (cadr box) 2)]
		[extent (caddr box)]
		[mode (cadddr box)]
		[rotation (cond [(not (eq? rot-x 0)) (rotate-x rot-x)]
				[(not (eq? rot-y 0)) (rotate-y rot-y)]
				[(not (eq? rot-z 0)) (rotate-z rot-z)]
				[else (translate 0 0 0)])]
		[world-matrix (mat4-mul translation rotation)])
	   (write-float* extent)
	   (write-float mode)
	   (write-float* world-matrix)
	   (write-float* rotation)))
       boxes)
  '())

(with-output-to-file "shape.blob" write-blob #:exists 'replace)
