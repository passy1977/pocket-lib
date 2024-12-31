
use crate::config::Config;
use crate::pods::device::Device;


#[allow(unused)] 
#[derive(Debug)]
pub struct Session {
    pub config: Config,
    pub init: bool
}

#[allow(unused)] 
impl Session {
    pub fn new(config_path: Option<String>) -> Option<Session> {
        match config_path {
            Option::None => Option::None,
            Option::Some(value) => {
                let mut session = Session { 
                    config: Config::new(Option::Some(value)),
                    init: false
                };


                session.init = true;
                Option::Some(session)   
            }
        }
    }

    
    pub fn register_device(_email: &String, _passwd: &String, _registration_json: &String) -> Option<Device> {
        Option::None
    }
}


#[cfg(test)]
mod tests {
    use super::pocket::init;

    const REGISTRATION : &str = r#"
    {
        "uuid": "de10b648-9e9c-4b63-96df-2aa662fe20ba",
        "user_uuid": "1281d20e-4e6f-43d8-9aa5-099039f36206",
        "host": "127.0.0.1",
        "host_pub_key": "fookey",
    }
    "#;

    #[test]
    fn it_works() {
        //init(REGISTRATION.to_string());
    }
}
