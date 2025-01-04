
use crate::database::Database;
pub trait DatabaseReadable<T, F> {
    
    fn read(&self, filter : &F) -> T;

}
