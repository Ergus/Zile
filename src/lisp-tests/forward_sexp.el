(forward-char)
(forward-char)
(forward-char)
(forward-char)
(forward-char)
(forward-char)
(insert ")")
(beginning-of-line)
(insert "(")
(beginning-of-line)
(forward-sexp)
(insert "a")
(save-buffer)
(save-buffers-kill-emacs)
