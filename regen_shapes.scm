#!/usr/bin/env racket
#lang racket

; colum-major storage
(define (mat4 a b c d
	      e f g h
	      i j k l
	      m n o p)
  (list a e i m
	b f j n
	c g k o
	d h l p))

(define (translate x y z)
  (mat4 1 0 0 x
	0 1 0 y
	0 0 1 z
	0 0 0 1))

(define (rotate-x angle)
  (let* ([P (sin (degrees->radians angle))]
	 [N (* -1 P)]
	 [C (cos (degrees->radians angle))])
    (mat4 1 0 0 0
	  0 C P 0
	  0 N C 0
	  0 0 0 1)))

(define (rotate-y angle)
  (let* ([P (sin (degrees->radians angle))]
	 [N (* -1 P)]
	 [C (cos (degrees->radians angle))])
    (mat4 C 0 N 0
	  0 1 0 0
	  P 0 C 0
	  0 0 0 1)))

(define (rotate-z angle)
  (let* ([P (sin (degrees->radians angle))]
	 [N (* -1 P)]
	 [C (cos (degrees->radians angle))])
    (mat4 C N 0 0
	  P C 0 0
	  0 0 1 0
	  0 0 0 1)))

(define (mat-row matrix i)
  (let ([y (remainder i 4)])
    (list (list-ref matrix (+ y 0))
	  (list-ref matrix (+ y 4))
	  (list-ref matrix (+ y 8))
	  (list-ref matrix (+ y 12)))))

(define (mat-col matrix i)
  (let ([x (* (quotient i 4) 4)])
    (list (list-ref matrix (+ x 0))
	  (list-ref matrix (+ x 1))
	  (list-ref matrix (+ x 2))
	  (list-ref matrix (+ x 3)))))

(define (mad a b c) (+ (* a b) c))

(define (dot lhs rhs)
  (if (or (null? lhs) (null? rhs)) 0
      (mad (car lhs) (car rhs) (dot (cdr lhs) (cdr rhs)))))

(define (mat4-mul lhs rhs)
  (map (lambda (i) (dot (mat-row lhs i) (mat-col rhs i))) (range 16)))

(define (write-uint32 x) (write-bytes (integer->integer-bytes x 4 #f)))
(define (write-uint32* x) (if (number? x) (write-uint32 x) (map write-uint32 x)))
(define (write-float x) (write-bytes (real->floating-point-bytes x 4)))
(define (write-float* x) (if (number? x) (write-float x) (map write-float x)))





(define spheres '((0 0 0 200)
		  (-50 -50 100 -150)
		  (100 100 100 -80)
		  (-10 -10 -100 -100)))

(define boxes '(((100 0 55) (0 45 0) (60 400 60) -1)
		((-100 -100 0) (0 0 45) (50 50 400) -1)))

(define vec4-size (* 4 4))
(define mat4-size (* vec4-size 4))
(define sphere-size vec4-size)
(define box-size (+ vec4-size (* mat4-size 2)))

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
