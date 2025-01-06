use crate::traits::pod::Pod;

#[allow(unused)] 
#[derive(Debug)]
#[derive(Clone)]
pub enum Status {
    Unactive = 1,
    Active = 0,
    Deleted = 2
}

#[allow(unused)] 
#[derive(Debug)]
#[derive(Clone)]
pub struct User {
    pub id: u64,
    pub uuid: String,
    pub status: Status
}

impl Pod for User {}

#[allow(unused)] 
impl User {
    pub fn new() -> Self {
        User {
            id: 0,
            uuid: "".to_owned(),
            status: Status::Active
        }
    }

    pub fn is_empty(&self) -> bool {
        self.id == 0 && self.uuid == ""
    }
}

