extern crate byteorder;

mod internal;

pub struct FlatVec<'a, A> {
    a: A,
    b: &'a u8,
}

pub struct FlatStr<'a> {
    a: &'a u8,
}
