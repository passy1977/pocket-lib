
use crate::database::{Database, Status};

pub trait DatabaseRead<T> {
    
    fn read(&mut self, database: &Database, filter : &T) -> Status;

}
