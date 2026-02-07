if [ -n "$DISPLAY" ]; then
(sleep 4 && /usr/bin/rice_set) &
fi
#pipewire-pulse &
mate_monitor -s &
