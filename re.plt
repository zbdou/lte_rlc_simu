# plot 15, 540ms

set xrange [0:100000]

set xlabel 'UMD PDU SN'
set ylabel 'T(ms)'

#set term post eps color solid enh
#set output '15_540.eps'

set term jpeg
set output '15_540.jpg'

set grid

set key right bottom

plot 'rr_15_540' w d title '(UW_Size, t-Reordering) = (2^15, 540ms)'

set xrange[0:4000]
set output '10_540.jpg'
plot 'rr_10_540' w d title '(UW_Size, t-Reordering) = (2^10, 540ms)'

#set xrange[0:100]
#set output '10_10.jpg'
#plot 'rr_10_10' w d title '(UW_Size, t-Reordering) = (2^10, 10ms)'
