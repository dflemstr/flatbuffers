struct FieldLoc {
    offset: u32,
    id: u16,
}

struct Builder {
    buffer: Vec<u8>,
    space: usize,
    vtable: Vec<i32>,
    min_align: u32,
    force_defaults: bool,
}

struct ObjectBuilder<'a> {
    builder: &'a mut Builder,
}

impl Builder {
    pub fn new() -> Self {
        Builder::with_buffer_capacity(1024)
    }

    pub fn with_buffer_capacity(capacity: usize) -> Self {
        Builder::with_buffer(Vec::with_capacity(capacity))
    }

    pub fn with_buffer(buffer: Vec<u8>) -> Self {
        let c = buffer.capacity();
        buffer.resize(c, 0);

        Builder {
            buffer: buffer,
            space: c,
            vtable: Vec::new(),
            min_align: 1,
            force_defaults: false,
        }
    }

    pub fn size(&self) -> usize {
        self.buffer.len()
    }

    pub fn object<'a>(&'a mut self, num_fields: u16) -> ObjectBuilder<'a> {
        self.vtable.clear();
        self.vtable.resize(num_fields as usize, 0);
        ObjectBuilder {
            builder: self
        }
    }

    fn prep(&mut self, size: u32, additional: u32) {
        if size > self.min_align {
            self.min_align = size;
        }

        let align_size =
            (!(self.buffer.capacity() - self.space + additional as usize) + 1) &
            (size as usize - 1);

        /*
        let total_size = align_size + size as usize + additional as usize;

        if self.space < total_size {
            self.buffer.reserve(total_size);
        }*/
    }

}
