(define (draw-polygon circ vert)
  (let ((side (/ circ vert))
        (angl (/ 360 vert)))
    (define (dp-inner count)
      (if (> count vert)
        vert
        (begin
          (tortoise-move side)
          (tortoise-turn angl)
          (dp-inner (+ 1 count)))))
    (dp-inner 0)))

(define (draw-polygon! circumference vertices)
  (let ((side (/ circumference vertices))
        (ang (/ 360 vertices)))
    (let iterate ((i 1))
      (if (<= i vertices)
        (begin
          (tortoise-move side)
          (tortoise-turn ang)
          (iterate (1+ i)))))))

(define (koch-line length depth)
  (if (zero? depth)
    (tortoise-move length)
    (let ((sub-length (/ length 3))
          (sub-depth (1- depth)))
      (for-each (lambda (angle)
                  (koch-line sub-length sub-depth)
                  (tortoise-turn angle))
                '(60 -120 60 0)))))

(define (snowflake length depth sign)
  (let iterate ((i 1))
    (if (<= i 3)
      (begin
        (koch-line length depth)
        (tortoise-turn (* sign -120))
        (iterate (1+ i))))))

