#!/bin/sh
# Flash a freertrtos image. This is a template: fill in the programmer command
# for your board and keep a known-good recovery image.
set -eu

IMAGE=${1:-build/thejas32/freertrtos_autopilot.elf}

if [ ! -f "$IMAGE" ]; then
    echo "image not found: $IMAGE" >&2
    exit 1
fi

if [ -z "${FREERTRTOS_PROGRAMMER:-}" ]; then
    echo "Set FREERTRTOS_PROGRAMMER to your programmer command." >&2
    echo "Example: FREERTRTOS_PROGRAMMER='openocd -f board.cfg -c \"program \$IMAGE verify reset exit\"'" >&2
    exit 1
fi

echo "flashing $IMAGE"
# shellcheck disable=SC2086
eval "$FREERTRTOS_PROGRAMMER"
