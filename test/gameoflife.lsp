(comment
"                        Game of life in TinyLisp                             "
"                                by Mibi88                                    "
"                                                                             "
"This software is licensed under the BSD-3-Clause license:                    "
"                                                                             "
"Copyright 2024 Mibi88                                                        "
"                                                                             "
"Redistribution and use in source and binary forms, with or without           "
"modification, are permitted provided that the following conditions are met:  "
"                                                                             "
"1. Redistributions of source code must retain the above copyright notice,    "
"this list of conditions and the following disclaimer.                        "
"                                                                             "
"2. Redistributions in binary form must reproduce the above copyright notice, "
"this list of conditions and the following disclaimer in the documentation    "
"and/or other materials provided with the distribution.                       "
"                                                                             "
"3. Neither the name of the copyright holder nor the names of its             "
"contributors may be used to endorse or promote products derived from this    "
"software without specific prior written permission.                          "
"                                                                             "
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  "
"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    "
"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   "
"ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE    "
"LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR          "
"CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         "
"SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS     "
"INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      "
"CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)      "
"ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   "
"POSSIBILITY OF SUCH DAMAGE.                                                  "
)

(strdef map (list
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                        ###                        "
    "                        # #                        "
    "                        # #                        "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
    "                                                   "
))

(numdef map_h (len map))
(numdef map_w (strlen (get map 0)))

(print "Map width:")
(print map_w)
(print "Map height:")
(print map_h)

(strdef line "")
(strdef newmap "")
(numdef living 0)

(fncdef update_line (params l i))
    (set living
        (+
            (+
                (+
                    (+ 
                        (= (strget (get map (if (< (- l 1) 0) (- map_h 1)
                            (% (- l 1) map_h))) (if (< (- i 1) 0) (- map_w 1)
                            (% (- i 1) map_w))) "#")
                        (= (strget (get map (if (< (- l 1) 0) (- map_h 1)
                            (% (- l 1) map_h))) (if (< i 0) (- map_w 1)
                            (% i map_w))) "#")
                    )
                    (= (strget (get map (if (< (- l 1) 0) (- map_h 1)
                        (% (- l 1) map_h))) (if (< (+ i 1) 0) (- map_w 1)
                        (% (+ i 1) map_w))) "#")
                )
                (+
                    (= (strget (get map (if (< l 0) (- map_h 1) (% l map_h)))
                        (if (< (- i 1) 0) (- map_w 1) (% (- i 1) map_w))) "#")
                    (= (strget (get map (if (< l 0) (- map_h 1) (% l map_h)))
                        (if (< (+ i 1) 0) (- map_w 1) (% (+ i 1) map_w))) "#")
                )
            )
            (+
                (+
                    (= (strget (get map (if (< (+ l 1) 0) (- map_h 1)
                        (% (+ l 1) map_h))) (if (< (- i 1) 0) (- map_w 1)
                        (% (- i 1) map_w))) "#")
                    (= (strget (get map (if (< (+ l 1) 0) (- map_h 1) (%
                        (+ l 1) map_h))) (if (< i 0) (- map_w 1) (% i map_w)))
                        "#")
                )
                (= (strget (get map (if (< (+ l 1) 0) (- map_h 1) (% (+ l 1)
                    map_h))) (if (< (+ i 1) 0) (- map_w 1) (% (+ i 1) map_w)))
                    "#")
            )
        )
    )
    (comment "(print (list i l living))")
    (set line (+ line
        (if (= living 3) "#" (if (= living 2) (strget (get map (if (< l 0)
            (- map_h 1) (% l map_h))) (if (< i 0) (- map_w 1) (% i map_w)))
            " "))
    ))
    (callif (< i (- map_w 1)) update_line l (+ i 1))
(defend)

(fncdef updatemap (params i))
    (set line "")
    (update_line i 0)
    (set newmap (++ newmap line))
    (callif (< i (- map_h 1)) updatemap (+ i 1))
(defend)

(fncdef drawmap (params i))
    (print (get map i))
    (callif (< i (- map_h 1)) drawmap (+ i 1))
(defend)

(fncdef mainloop (params i m))
    (print "-------------------")
    (print "Iteration:")
    (print i)
    (print "-------------------")
    (drawmap 0)
    (set line "")
    (update_line 0 0)
    (set newmap line)
    (updatemap 1)
    (set map newmap)
    (callif (< i m) mainloop (+ i 1) m)
(defend)

(mainloop 0 110)

