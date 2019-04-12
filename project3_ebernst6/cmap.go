package main

type ChannelMap struct {
     words map[string]int
     ask_buf chan string
     add_buf chan string
     answer_buf chan int
     reduce_buf chan ReduceFunc

     reduce_ans_buf chan string
     reduce_int_buf chan int
     quit_chan chan int
}

func (c *ChannelMap) Listen() {
    //loop forever
    for {
         //see which channels have data
        select {
            //if add_buf had data, pull a word out and add it to the map
            case word := <- c.add_buf:
                c.words[word]++
            //if ask_buf has data, pull out a word, and send its mapped val to answer_buf for output
            case word := <- c.ask_buf:
                c.answer_buf <- c.words[word]
            //if reduce_buf has data, run the functor on all words
            case fun := <- c.reduce_buf:
                accum := " "
                anum := 0

                for word, val := range c.words {
                    accum, anum = fun(accum, anum, word, val)
                }

                //output to reduce
                c.reduce_ans_buf <- accum
                c.reduce_int_buf <- anum

            //if we get something in the quit channel, gtfo
            case quit := <- c.quit_chan:
                _ = quit
                return
        }
    }
}

func (c *ChannelMap) Stop() {
    c.quit_chan <- 1
}

func (c *ChannelMap) Reduce(functor ReduceFunc, accum_str string, accum_int int) (string, int) {
    c.reduce_buf <- functor

	_ = accum_str
	_ = accum_int
	return <-c.reduce_ans_buf, <-c.reduce_int_buf
}
// adds word to the ask buffer
func (c *ChannelMap) AddWord(word string) {
	c.add_buf <- word
}

//adds words to the ask buffer, then waits for the answer_buffer and returns that value
func (c *ChannelMap) GetCount(word string) int {
	c.ask_buf <- word

	return <- c.answer_buf
}

func NewChannelMap() *ChannelMap {
     	return &ChannelMap {
        words : make (map[string] int),
        ask_buf :  make(chan string, ASK_BUFFER_SIZE),
        add_buf : make(chan string, ADD_BUFFER_SIZE),
        answer_buf : make(chan int, ASK_BUFFER_SIZE),
        reduce_buf : make(chan ReduceFunc, REDUCE_BUFFER_SIZE),
        reduce_ans_buf : make (chan string, REDUCE_BUFFER_SIZE),
        reduce_int_buf : make(chan int, REDUCE_BUFFER_SIZE),
        quit_chan : make(chan int)  }
}
