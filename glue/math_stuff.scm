#lang racket

(provide
 translate
 rotate-x
 rotate-y
 rotate-z
 mat4-mul)

;; We use column-major storage, whereas what looks natural below would
;; be row-major, so this transposes it:
(define (mat4 a b c d
	      e f g h
	      i j k l
	      m n o p)
  (list a e i m
	b f j n
	c g k o
	d h l p))

;; Create a translation matrix:
(define (translate x y z)
  (mat4 1 0 0 x
	0 1 0 y
	0 0 1 z
	0 0 0 1))

;; Create a rotation matrix around the x axis:
(define (rotate-x angle)
  (let* ([P (sin (degrees->radians angle))]
	 [N (* -1 P)]
	 [C (cos (degrees->radians angle))])
    (mat4 1 0 0 0
	  0 C P 0
	  0 N C 0
	  0 0 0 1)))

;; Create a rotation matrix around the y axis:
(define (rotate-y angle)
  (let* ([P (sin (degrees->radians angle))]
	 [N (* -1 P)]
	 [C (cos (degrees->radians angle))])
    (mat4 C 0 N 0
	  0 1 0 0
	  P 0 C 0
	  0 0 0 1)))

;; Create a rotation matrix around the z axis:
(define (rotate-z angle)
  (let* ([P (sin (degrees->radians angle))]
	 [N (* -1 P)]
	 [C (cos (degrees->radians angle))])
    (mat4 C N 0 0
	  P C 0 0
	  0 0 1 0
	  0 0 0 1)))

;; Helper functions for matrix multiplication:
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

;; Multiply two matrices:
(define (mat4-mul lhs rhs)
  (map (lambda (i) (dot (mat-row lhs i) (mat-col rhs i))) (range 16)))
