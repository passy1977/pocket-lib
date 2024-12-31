
use crate::config::Config;
use crate::pods::device::Device;


#[allow(unused)] 
#[derive(Debug)]
pub struct Session {
    pub config: Option<Config>,
    pub init: bool
}

#[allow(unused)] 
impl Session {
    pub fn new(config_json: Option<String>, config_path: Option<String>) -> Option<Session> {
        
        match (config_json, config_path) {
            (json @ None, _) => None,
            (json, path) if !json.clone().unwrap().is_empty() => {
                let mut session = Session { 
                    config: Config::new(json.unwrap(), path),
                    init: false
                };


                Some(session) 
            },
            
            
            
            // Option::None => Option::None,
            // Option::Some(value) => {
            //     let mut session = Session { 
            //         config: Config::new(Option::Some(value)),
            //         init: false
            //     };


            //     session.init = true;
            //     Option::Some(session)   
            // }
            (_, _) => None,
        }
    }

    
    pub fn register_device(_email: &String, _passwd: &String, _registration_json: &String) -> Option<Device> {
        None
    }
}


#[cfg(test)]
mod tests {

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
