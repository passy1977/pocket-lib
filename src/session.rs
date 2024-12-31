
use crate::config::Config;
use crate::pods::device::Device;


#[allow(unused)] 
#[derive(Debug)]
pub struct Session {
    pub config: Option<Config>,
    pub device: Option<Device>,
    pub init: bool
}

#[allow(unused)] 
impl Session {
    pub fn new(config_json: Option<String>, config_path: Option<String>) -> Result<Session, &'static str> {
        
        match (config_json, config_path) {
            (json @ None, _) => Err("Config json not defined"),
            (json, path) if !json.clone().unwrap().is_empty() => {

                let mut session = Session { 
                    config: None,
                    device: None,
                    init: false
                };
                
                
                
                if let Ok(conf) = Config::new(&mut session, json.unwrap(), path) {
                    session.config = Some(conf);
                    session.init = true;
                    Ok(session) 
                } else {
                    Err("Config error")
                }



            },
            
            (_, _) => Err("Un handled error"),
        }
    }

    
    pub fn register_device(_email: &String, _passwd: &String, _registration_json: &String) -> Option<Device> {
        None
    }
}


#[cfg(test)]
mod tests {
    use super::Session;


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
        let _session = Session::new(
            Option::Some(REGISTRATION.to_string()), 
            Option::None
        );
        
    }
}
