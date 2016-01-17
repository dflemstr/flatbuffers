use byteorder;

use std::default;
use std::mem;

type ORDER = byteorder::LittleEndian;

pub struct Struct<'a>(&'a [u8], usize);
pub struct Table<'a>(&'a [u8], usize);

impl<'a> Table<'a> {
    fn offset(&self, slot: u16) -> Option<u16> {
        use byteorder::ByteOrder;

        let Table(data, table_pos) = *self;
        let vtable_offset = ORDER::read_i32(&data[table_pos..]);
        let vtable_pos = if vtable_offset < 0 {
            table_pos + ((-vtable_offset) as usize)
        } else {
            table_pos - (vtable_offset as usize)
        };

        let vtable_size = ORDER::read_u16(&data[vtable_pos..]) as usize;

        let slot_offset = (slot + 1) as usize * mem::size_of::<u16>();
        if slot_offset < vtable_size {
            Some(ORDER::read_u16(&data[vtable_pos + slot_offset..]))
        } else {
            None
        }
    }
}

trait Get<A> {
    fn get(&self, offset: u16) -> A;
    fn get_with_default(&self, offset: u16, default: A) -> A;
}

macro_rules! impl_primitive_get {
    ( $t:ty, $default:expr) => {
        impl<'a> Get<$t> for Struct<'a> {
            fn get(&self, offset: u16) -> $t {
                $default
            }

            fn get_with_default(&self, offset: u16, default: $t) -> $t {
                default
            }
        }

        impl<'a> Get<$t> for Table<'a> {
            fn get(&self, offset: u16) -> $t {
                $default
            }

            fn get_with_default(&self, offset: u16, default: $t) -> $t {
                default
            }
        }
    }
}

impl_primitive_get!(bool, false);
impl_primitive_get!(u8, 0);
impl_primitive_get!(u16, 0);
impl_primitive_get!(u32, 0);
impl_primitive_get!(u64, 0);
impl_primitive_get!(i8, 0);
impl_primitive_get!(i16, 0);
impl_primitive_get!(i32, 0);
impl_primitive_get!(i64, 0);
impl_primitive_get!(f32, 0.0);
impl_primitive_get!(f64, 0.0);
