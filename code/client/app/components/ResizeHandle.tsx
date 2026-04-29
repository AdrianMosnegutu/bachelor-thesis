"use client";

import { Separator } from "react-resizable-panels";

interface ResizeHandleProps {
  direction?: "horizontal" | "vertical";
  className?: string;
}

export default function ResizeHandle({ direction = "horizontal", className = "" }: ResizeHandleProps) {
  return (
    <Separator
      className={`
        flex items-center justify-center
        bg-border hover:bg-zinc-400 dark:hover:bg-zinc-600 transition-colors
        ${direction === "horizontal" ? "w-1 h-full cursor-col-resize" : "h-1 w-full cursor-row-resize"}
        ${className}
      `}
    />
  );
}
