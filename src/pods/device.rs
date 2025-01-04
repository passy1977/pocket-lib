use crate::traits::pod::Pod;


#[repr(C)]
#[allow(unused)] 
#[derive(Clone)]
#[derive(Debug)]
pub enum Status {
    Unactive = 1,
    Active = 0,
    Deleted = 2,
    Invalidated = 3
}

#[allow(unused)] 
#[derive(Clone)]
#[derive(Debug)]
pub struct Device {
    pub id: u64,
    pub uuid: String,
    pub user_uuid: String,
    pub host: String,
    pub host_pub_key: String,
    pub timestamp_last_update: u64,
    pub timestamp_creation: u64,
    pub status: Status
}

impl Pod for Device {}

#[allow(unused)] 
impl Device {
    pub fn new() -> Self {
        Device {
            id: 0,
            uuid: "".to_owned(),
            user_uuid: "".to_owned(),
            host: "".to_owned(),
            host_pub_key: "".to_owned(),
            timestamp_last_update: 0,
            timestamp_creation: 0,
            status: Status::Active
        }
    }
}