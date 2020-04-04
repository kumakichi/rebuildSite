package main

import (
	"flag"
	"fmt"
	"net/http"
	"os"

	"github.com/gobuffalo/packr"
)

var (
	dir  = flag.String("dir", "./pc_output", "static files directory")
	help = flag.Bool("help", false, "show help info")
	port = flag.Int("port", 3000, "serve port")
)

func init() {
	flag.Parse()
	if *help {
		flag.PrintDefaults()
		os.Exit(0)
	}
}

func main() {
	box := packr.NewBox(*dir)

	http.Handle("/", http.FileServer(box))
	host := fmt.Sprintf(":%d", *port)
	fmt.Printf("listen on %s\n", host)
	http.ListenAndServe(host, nil)
}
